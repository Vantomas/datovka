/*
 * Copyright (C) 2014-2016 CZ.NIC
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations including
 * the two.
 */

#include <QDateTime>
#include <QDir>
#include <QtTest/QtTest>

#include "src/io/account_db.h"
#include "src/io/isds_sessions.h"
#include "src/settings/preferences.h"
#include "src/worker/task_send_message.h"
#include "tests/helper_qt.h"
#include "tests/test_task_send_message.h"

class TestTaskSendMessage : public QObject {
	Q_OBJECT

public:
	explicit TestTaskSendMessage(qint64 &sentMsgId);

	~TestTaskSendMessage(void);

private slots:
	void initTestCase(void);

	void cleanupTestCase(void);

	void sendMessage(void);

private:
	static
	IsdsMessage buildMessage(const QString &recipBox);

	const bool m_testing; /*!< Testing account. */
	const enum MessageDbSet::Organisation m_organisation; /*!< Database organisation. */

	const QString m_connectionPrefix; /*!< SQL connection prefix. */

	const QString m_testPath; /*!< Test path. */
	QDir m_testDir;  /*!< Directory containing testing data. */

	const QString m_credFName; /*!< Credentials file name. */

	/* Log-in using user name and password. */
	LoginCredentials m_sender; /*!< Sender credentials. */
	LoginCredentials m_recipient; /*!< Recipient credentials. */

	MessageDbSet *m_senderDbSet; /*!< Databases. */

	QString m_confSubDirBackup; /*!< Backup for the configuration directory. */

	qint64 &m_sentMsgId; /*!< Identifier ow newly sent message. */
};

#define printCredentials(cred) \
	fprintf(stderr, "Credentials '" #cred "': '%s', '%s', '%s'\n", \
	    (cred).boxName.toUtf8().constData(), \
	    (cred).userName.toUtf8().constData(), \
	    (cred).pwd.toUtf8().constData())

TestTaskSendMessage::TestTaskSendMessage(qint64 &sentMsgId)
    : m_testing(true),
    m_organisation(MessageDbSet::DO_YEARLY),
    m_connectionPrefix(QLatin1String("GLOBALDBS")),
    m_testPath(QDir::currentPath() + QDir::separator() + QLatin1String("_test_dir")),
    m_testDir(m_testPath),
    m_credFName(QLatin1String(CREDENTIALS_FNAME)),
    m_sender(),
    m_recipient(),
    m_senderDbSet(NULL),
    m_confSubDirBackup(globPref.confSubdir),
    m_sentMsgId(sentMsgId)
{
	/* Set configuration subdirectory to some value. */
	globPref.confSubdir = QLatin1String(".datovka_test");
}

TestTaskSendMessage::~TestTaskSendMessage(void)
{
	/* Just in case. */
	delete m_senderDbSet; m_senderDbSet = NULL;

	/* Restore original value. */
	globPref.confSubdir = m_confSubDirBackup;
}

void TestTaskSendMessage::initTestCase(void)
{
	bool ret;

	/* Create empty working directory. */
	m_testDir.removeRecursively();
	QVERIFY(!m_testDir.exists());
	m_testDir.mkpath(".");
	QVERIFY(m_testDir.exists());

	/* Load credentials. */
	ret = m_sender.loadLoginCredentials(m_credFName, 1);
	if (!ret) {
		QSKIP("Failed to load login credentials. Skipping remaining tests.");
	}
	QVERIFY(ret);
	QVERIFY(!m_sender.userName.isEmpty());
	QVERIFY(!m_sender.pwd.isEmpty());

	ret = m_recipient.loadLoginCredentials(m_credFName, 2);
	if (!ret) {
		QSKIP("Failed to load login credentials. Skipping remaining tests.");
	}
	QVERIFY(ret);
	QVERIFY(!m_recipient.userName.isEmpty());
	QVERIFY(!m_recipient.pwd.isEmpty());

	/* Access message database. */
	m_senderDbSet = MessageDbSet::createNew(m_testPath, m_sender.userName,
	    m_testing, m_organisation, m_connectionPrefix,
	    MessageDbSet::CM_CREATE_EMPTY_CURRENT);
	if (m_senderDbSet == NULL) {
		QSKIP("Failed to open message database.");
	}
	QVERIFY(m_senderDbSet != NULL);

	/*
	 * Create accounts database and open it. It is required by the task.
	 */
	QVERIFY(globAccountDbPtr == NULL);
	globAccountDbPtr = new (::std::nothrow) AccountDb("accountDb");
	if (globAccountDbPtr == NULL) {
		QSKIP("Cannot create accounts database.");
	}
	QVERIFY(globAccountDbPtr != NULL);
	ret = globAccountDbPtr->openDb(m_testPath + QDir::separator() + "messages.shelf.db");
	if (!ret) {
		QSKIP("Cannot open account database.");
	}
	QVERIFY(ret);

	/* Log into ISDS. */
	struct isds_ctx *ctx = globIsdsSessions.session(m_sender.userName);
	if (!globIsdsSessions.holdsSession(m_sender.userName)) {
		QVERIFY(ctx == NULL);
		ctx = globIsdsSessions.createCleanSession(m_sender.userName,
		    globPref.isds_download_timeout_ms);
	}
	if (ctx == NULL) {
		QSKIP("Cannot obtain communication context.");
	}
	QVERIFY(ctx != NULL);
	isds_error err = isdsLoginUserName(ctx, m_sender.userName,
	    m_sender.pwd, m_testing);
	if (err != IE_SUCCESS) {
		QSKIP("Error connection into ISDS.");
	}
	QVERIFY(err == IE_SUCCESS);
}

void TestTaskSendMessage::cleanupTestCase(void)
{
	delete m_senderDbSet; m_senderDbSet = NULL;

	/* Delete account database. */
	delete globAccountDbPtr; globAccountDbPtr = NULL;

	/* The configuration directory should be non-existent. */
	QVERIFY(!QDir(globPref.confDir()).exists());

	/* Delete testing directory. */
	m_testDir.removeRecursively();
	QVERIFY(!m_testDir.exists());
}

void TestTaskSendMessage::sendMessage(void)
{
	TaskSendMessage *task;

	QVERIFY(!m_sender.userName.isEmpty());

	QVERIFY(m_senderDbSet != NULL);

	QVERIFY(globIsdsSessions.isConnectedToIsds(m_sender.userName));
	struct isds_ctx *ctx = globIsdsSessions.session(m_sender.userName);
	QVERIFY(ctx != NULL);
	QVERIFY(globIsdsSessions.isConnectedToIsds(m_sender.userName));

	QString transactionId(QLatin1String("some_id"));
	QString recipientName(QLatin1String("recipient name"));
	QString recipientAddress(QLatin1String("recipient address"));
	bool isPDZ = false;

	/* Sending empty message must fail. */
	task = new (::std::nothrow) TaskSendMessage(m_sender.userName,
	    m_senderDbSet, transactionId, IsdsMessage(), recipientName,
	    recipientAddress, isPDZ);
	QVERIFY(task != NULL);
	task->setAutoDelete(false);

	task->run();

	QVERIFY(task->m_resultData.result == TaskSendMessage::SM_ERR);

	delete task; task = NULL;

	/* Sending message should succeed. */
	task = new (::std::nothrow) TaskSendMessage(m_sender.userName,
	    m_senderDbSet, transactionId, buildMessage(m_recipient.boxName),
	    recipientName, recipientAddress, isPDZ);
	QVERIFY(task != NULL);
	task->setAutoDelete(false);

	task->run();

	QVERIFY(task->m_resultData.result == TaskSendMessage::SM_SUCCESS);

	QVERIFY(task->m_resultData.dmId != 0);
	m_sentMsgId = task->m_resultData.dmId;

	delete task; task = NULL;
}

IsdsMessage TestTaskSendMessage::buildMessage(const QString &recipBox)
{
	IsdsMessage msg;

	QString dateTime(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

	msg.envelope.dbIDRecipient = recipBox;
	msg.envelope.dmAnnotation = QLatin1String("sending-test-") + dateTime;

	IsdsDocument document;
	document.dmFileDescr = QLatin1String("priloha.txt");
	document.data = QString("Priloha vygenerovana %1.").arg(dateTime).toUtf8();

	msg.documents.append(document);

	return msg;
}

QObject *newTestTaskSendMessage(qint64 &sentMsgId)
{
	return new (::std::nothrow) TestTaskSendMessage(sentMsgId);
}

//QTEST_MAIN(TestTaskSendMessage)
#include "test_task_send_message.moc"
