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

#include <QByteArray>
#include <QFile>
#include <QThread>

#include "src/gui/dlg_import_zfo.h" /* TODO -- Remove this dependency. */
#include "src/io/dbs.h"
#include "src/io/isds_sessions.h"
#include "src/log/log.h"
#include "src/worker/message_emitter.h"
#include "src/worker/task_authenticate_message.h"
#include "src/worker/task_import_zfo.h"

TaskImportZfo::TaskImportZfo(const QList<AccountData> &accounts, const QString &fileName,
    enum ZfoType type, bool authenticate)
    : m_fileName(fileName),
    m_result(IMP_ERR),
    m_isdsError(),
    m_isdsLongError(),
    m_resultDesc(),
    m_accounts(accounts),
    m_zfoType(type),
    m_auth(authenticate)
{
	Q_ASSERT(!m_fileName.isEmpty());
	Q_ASSERT(!m_accounts.isEmpty());

	if (ZT_UKNOWN == m_zfoType) {
		m_zfoType = determineFileType(m_fileName);
	}
}

void TaskImportZfo::run(void)
{
	if (m_fileName.isEmpty()) {
		Q_ASSERT(0);
		return;
	}

	if (m_accounts.isEmpty()) {
		Q_ASSERT(0);
		return;
	}

	if (ZT_UKNOWN == m_zfoType) {
		m_result = IMP_DATA_ERROR;
		return;
	}

	logDebugLv0NL("Starting import ZFO task in thread '%p'",
	    (void *) QThread::currentThreadId());

	/* ### Worker task begin. ### */

	if (ZT_MESSAGE == m_zfoType) {
//	TODO
	} else if (ZT_DELIVERY_INFO == m_zfoType) {
		m_result = importDeliveryZfo(m_accounts, m_fileName, m_auth,
		    m_resultDesc);
	}

	emit globMsgProcEmitter.progressChange(PL_IDLE, 0);

	/* ### Worker task end. ### */

	logDebugLv0NL("Import ZFO task finished in thread '%p'",
	    (void *) QThread::currentThreadId());
}

enum TaskImportZfo::ZfoType TaskImportZfo::determineFileType(
    const QString &fileName)
{
	debugFuncCall();

	ZfoType zfoType = ZT_UKNOWN;

	if (fileName.isEmpty()) {
		Q_ASSERT(0);
		return zfoType;
	}

	struct isds_ctx *dummySession = isds_ctx_create();
	if (NULL == dummySession) {
		logErrorNL("%s", "Cannot create dummy ISDS session.");
		return zfoType;
	}

	struct isds_message *message = loadZfoFile(dummySession, fileName,
	    ImportZFODialog::IMPORT_MESSAGE_ZFO);
	if (NULL != message) {
		zfoType = ZT_MESSAGE;
	} else {
		message = loadZfoFile(dummySession, fileName,
		    ImportZFODialog::IMPORT_DELIVERY_ZFO);
		if(NULL != message) {
			zfoType = ZT_DELIVERY_INFO;
		} else {
			zfoType = ZT_UKNOWN;
		}
	}

	isds_message_free(&message);
	isds_ctx_free(&dummySession);

	return zfoType;
}

/*!
 * @brief Performs message file authentication.
 *
 * @param[in]  userName      Account identifier (user login name).
 * @param[in]  fileName      Full name of message file.
 * @param[out] isdsError     Error description.
 * @param[out] isdsLongError Long error description.
 * @return Authentication result.
 */
static
TaskAuthenticateMessage::Result authenticateMessageFile(const QString &userName,
    const QString &fileName, QString &isdsError, QString &isdsLongError)
{
	Q_ASSERT(!userName.isEmpty());
	Q_ASSERT(!fileName.isEmpty());

	QFile file(fileName);
	if (!file.exists()) {
		return TaskAuthenticateMessage::AUTH_DATA_ERROR;
	}

	QByteArray data(file.readAll());
	file.close();

	return TaskAuthenticateMessage::authenticateMessage(userName, data,
	    isdsError, isdsLongError);
}

/*!
 * @brief Tries to import a single delivery info ZFO file into a single account.
 *
 * @param[in]  acnt          Account to try to insert delivery info into.
 * @param[in]  message       Parsed delivery info.
 * @param[in]  dmId          Message identifier.
 * @param[in]  deliveryTime  Message delivery time.
 * @param[in]  fileName      If supplied then the file is going to be
 *                           authenticated on ISDS server.
 * @param[out] isdsError     Error description.
 * @param[out] isdsLongError Long error description.
 * @param[out] resultDesc    Result description.
 * @returns Error identifier.
 */
static
enum TaskImportZfo::Result importDeliveryZfoSingle(
    const TaskImportZfo::AccountData &acnt, const struct isds_message *message,
    qint64 dmId, const QDateTime &deliveryTime, const QString &fileName,
    QString &isdsError, QString &isdsLongError, QString &resultDesc)
{
	Q_ASSERT(acnt.isValid());
	Q_ASSERT((NULL != message) && (NULL != message->envelope));

	MessageDb *messageDb = acnt.messageDbSet->accessMessageDb(
	    deliveryTime, false);
	const QString accountName(
	    AccountModel::globAccounts[acnt.userName].accountName());
	if ((NULL == messageDb) ||
	    (-1 == messageDb->msgsStatusIfExists(dmId))) {
		/* Corresponding message does not exist in database. */
		resultDesc = QObject::tr("This file (delivery info) has not "
		    "been inserted into database because there isn't any "
		    "related message with id '%1' in the databases.").arg(dmId);
		return TaskImportZfo::IMP_DB_MISSING_MSG;
	}

	if (messageDb->isDeliveryInfoRawDb(dmId)) {
		resultDesc = QObject::tr("Delivery info for message '%1' "
		    "already exists in the local database, account '%2'.").
		    arg(dmId).arg(accountName);
		return TaskImportZfo::IMP_DB_EXISTS;
	}

	if (!fileName.isEmpty()) {
		TaskAuthenticateMessage::Result ret = authenticateMessageFile(
		    acnt.userName, fileName, isdsError, isdsLongError);
		switch (ret) {
		case TaskAuthenticateMessage::AUTH_SUCCESS:
			/* Continue with execution. */
			break;
		case TaskAuthenticateMessage::AUTH_DATA_ERROR:
			resultDesc = QObject::tr("Couldn't read data from "
			    "file for authentication on the ISDS server.");
			return TaskImportZfo::IMP_DATA_ERROR;
			break;
		case TaskAuthenticateMessage::AUTH_ISDS_ERROR:
			resultDesc = QObject::tr(
			    "Error contacting ISDS server.");
			return TaskImportZfo::IMP_ISDS_ERROR;
			break;
		default:
			resultDesc = QObject::tr("Delivery info for message "
			    "'%1' could not be authenticated by ISDS server.").
			    arg(dmId);
			return TaskImportZfo::IMP_AUTH_ERR;
			break;
		}
	}

	if (Q_SUCCESS !=
	    Task::storeDeliveryInfo(true, *(acnt.messageDbSet), message)) {
		resultDesc = QObject::tr("File has not been imported because "
		    "an error was detected during insertion process.");
		return TaskImportZfo::IMP_DB_INS_ERR;
	}

	resultDesc += QObject::tr(
	    "Imported delivery info for message '%1', account '%2'.").
	    arg(dmId).arg(accountName);
	return TaskImportZfo::IMP_SUCCESS;
}

enum TaskImportZfo::Result TaskImportZfo::importDeliveryZfo(
    const QList<TaskImportZfo::AccountData> &accounts, const QString &fileName,
    bool authenticate, QString &resultDesc)
{
	struct isds_message *message = NULL;

	{
		struct isds_ctx *dummy_session = isds_ctx_create();
		if (NULL == dummy_session) {
			logErrorNL("%s\n", "Cannot create dummy ISDS session.");
			return IMP_ERR;
		}

		message = loadZfoFile(dummy_session, fileName,
		    ImportZFODialog::IMPORT_DELIVERY_ZFO);
		isds_ctx_free(&dummy_session);
	}

	if ((NULL == message) || (NULL == message->envelope)) {
		logErrorNL("Wrong format of file '%s', expected delivery info.",
		    fileName.toUtf8().constData());
		resultDesc = QObject::tr("Wrong ZFO format. This file does not "
		    "contain correct data for import.");
		isds_message_free(&message);
		return IMP_DATA_ERROR;
	}

	qint64 dmId = QString(message->envelope->dmID).toLongLong();
	QDateTime deliveryTime =
	    timevalToDateTime(message->envelope->dmDeliveryTime);
	if (!deliveryTime.isValid()) {
		isds_message_free(&message);
		return IMP_DATA_ERROR;
	}

	QString importedDescr;
	QString existingDescr;
	QString errorDescr;
	QString isdsError, isdsLongError;
	bool imported = false;
	bool exists = false;

	foreach (const AccountData &acnt, accounts) {
		isdsError.clear();
		isdsLongError.clear();
		resultDesc.clear();
		enum Result res = importDeliveryZfoSingle(acnt,
		    message, dmId, deliveryTime,
		    authenticate ? fileName : QString(),
		    isdsError, isdsLongError, resultDesc);

		if (IMP_SUCCESS == res) {
			imported = true;
			importedDescr += resultDesc + "<br/>";
		} else if (IMP_DB_EXISTS == res) {
			exists = true;
			existingDescr += resultDesc + "<br/>";
		} else {
			errorDescr += resultDesc + "<br/>";
		}
	}

	if (imported) {
		resultDesc = importedDescr;
		return IMP_SUCCESS;
	} else if (exists) {
		resultDesc = existingDescr;
		return IMP_DB_EXISTS;
	} else {
		resultDesc = errorDescr;
		return IMP_ERR;
	}
}
