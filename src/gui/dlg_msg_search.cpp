/*
 * Copyright (C) 2014-2017 CZ.NIC
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

#include "src/common.h"
#include "src/gui/dlg_msg_search.h"
#include "src/io/message_db.h"
#include "src/io/tag_db.h"
#include "src/log/log.h"
#include "src/settings/accounts.h"
#include "src/views/table_home_end_filter.h"

#define COL_USER_NAME 0
#define COL_MESSAGE_ID 1
#define COL_DELIVERY_YEAR 2
#define COL_MESSAGE_TYPE 3
#define COL_ANNOTATION 4
#define COL_SENDER 5
#define COL_RECIPIENT 6

DlgMsgSearch::DlgMsgSearch(
    const QList< QPair <QString, MessageDbSet *> > messageDbSetList,
    const QString &userName,
    QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f),
    m_messageDbSetList(messageDbSetList),
    m_userName(userName)
{
	setupUi(this);
	/* Set default line height for table views/widgets. */
	resultsTableWidget->setNarrowedLineHeight();

	initSearchWindow();
}


/* ========================================================================= */
/*
 * Init message search dialog
 */
void DlgMsgSearch::initSearchWindow(void)
/* ========================================================================= */
{
	this->infoTextLabel->setText(
	   tr("Here it is possible to search for messages according to "
	   "supplied criteria. You can search for messages in selected "
	   "account or in all accounts. Double clicking on a found message "
	   "will change focus to the selected message in the application "
	   "window. Note: You can view additional information when hovering "
	   "your mouse cursor over the message ID."));

	Q_ASSERT(!m_userName.isEmpty());

	/* set account name and user name to label */
	QString accountName = globAccounts[m_userName].accountName() + " (" +
	    m_userName + ")";
	this->currentAccountName->setText(accountName);

	this->tooMuchFields->setStyleSheet("QLabel { color: red }");
	this->tooMuchFields->hide();

	/* is only one account available */
	if (m_messageDbSetList.count() <= 1) {
		this->searchAllAcntCheckBox->setEnabled(false);
	}

	this->resultsTableWidget->setColumnCount(7);
	this->resultsTableWidget->setHorizontalHeaderItem(COL_USER_NAME, new QTableWidgetItem(tr("Account")));
	this->resultsTableWidget->setHorizontalHeaderItem(COL_MESSAGE_ID, new QTableWidgetItem(tr("Message ID")));
	this->resultsTableWidget->setHorizontalHeaderItem(COL_ANNOTATION, new QTableWidgetItem(tr("Subject")));
	this->resultsTableWidget->setHorizontalHeaderItem(COL_SENDER, new QTableWidgetItem(tr("Sender")));
	this->resultsTableWidget->setHorizontalHeaderItem(COL_RECIPIENT, new QTableWidgetItem(tr("Recipient")));
	this->resultsTableWidget->setHorizontalHeaderItem(COL_DELIVERY_YEAR, new QTableWidgetItem(tr("Delivery Year")));
	this->resultsTableWidget->setHorizontalHeaderItem(COL_MESSAGE_TYPE, new QTableWidgetItem(tr("Message Type")));

	/* Hide column with delivery time and message type. */
	this->resultsTableWidget->setColumnHidden(COL_DELIVERY_YEAR, true);
	this->resultsTableWidget->setColumnHidden(COL_MESSAGE_TYPE, true);

	connect(this->searchReceivedMsgCheckBox, SIGNAL(clicked()),
	    this, SLOT(checkInputFields()));
	connect(this->searchSentMsgCheckBox, SIGNAL(clicked()),
	    this, SLOT(checkInputFields()));
	connect(this->messageIdLineEdit, SIGNAL(textChanged(QString)),
	    this, SLOT(checkInputFields()));
	connect(this->senderDbIdLineEdit, SIGNAL(textChanged(QString)),
	    this, SLOT(checkInputFields()));
	connect(this->senderNameLineEdit, SIGNAL(textChanged(QString)),
	    this, SLOT(checkInputFields()));
	connect(this->recipientDbIdLineEdit, SIGNAL(textChanged(QString)),
	    this, SLOT(checkInputFields()));
	connect(this->recipientNameLineEdit, SIGNAL(textChanged(QString)),
	    this, SLOT(checkInputFields()));
	connect(this->subjectLineEdit, SIGNAL(textChanged(QString)),
	    this, SLOT(checkInputFields()));
	connect(this->toHandsLineEdit, SIGNAL(textChanged(QString)),
	    this, SLOT(checkInputFields()));
	connect(this->addressLineEdit, SIGNAL(textChanged(QString)),
	    this, SLOT(checkInputFields()));
	connect(this->senderRefNumLineEdit, SIGNAL(textChanged(QString)),
	    this, SLOT(checkInputFields()));
	connect(this->senderFileMarkLineEdit, SIGNAL(textChanged(QString)),
	    this, SLOT(checkInputFields()));
	connect(this->recipientRefNumLineEdit, SIGNAL(textChanged(QString)),
	    this, SLOT(checkInputFields()));
	connect(this->recipientFileMarkLineEdit, SIGNAL(textChanged(QString)),
	    this, SLOT(checkInputFields()));
	connect(this->tagLineEdit, SIGNAL(textChanged(QString)),
	    this, SLOT(checkInputFields()));
	connect(this->searchPushButton, SIGNAL(clicked()), this,
	    SLOT(searchMessages()));
	connect(this->resultsTableWidget,
	    SIGNAL(itemSelectionChanged()), this,
	    SLOT(setFirtsColumnActive()));
	connect(this->resultsTableWidget,
	    SIGNAL(cellDoubleClicked(int,int)), this,
	    SLOT(getSelectedMsg(int, int)));

	this->resultsTableWidget->
	    setEditTriggers(QAbstractItemView::NoEditTriggers);

	this->resultsTableWidget->installEventFilter(
	    new TableHomeEndFilter(this));
}


/* ========================================================================= */
/*
 * Check dialogue elements and set search button enable/disable
 */
void DlgMsgSearch::checkInputFields(void)
/* ========================================================================= */
{
	//debugSlotCall();

	bool isAnyMsgTypeChecked = true;

	this->searchPushButton->setEnabled(false);
	this->tooMuchFields->hide();

	/* is any message type checked? */
	if (!this->searchReceivedMsgCheckBox->isChecked() &&
	    !this->searchSentMsgCheckBox->isChecked()) {
		isAnyMsgTypeChecked = false;
	}

	/* search via message ID */
	if (!this->messageIdLineEdit->text().isEmpty()) {
		this->subjectLineEdit->setEnabled(false);
		this->senderDbIdLineEdit->setEnabled(false);
		this->senderNameLineEdit->setEnabled(false);
		this->senderRefNumLineEdit->setEnabled(false);
		this->senderFileMarkLineEdit->setEnabled(false);
		this->recipientDbIdLineEdit->setEnabled(false);
		this->recipientNameLineEdit->setEnabled(false);
		this->recipientRefNumLineEdit->setEnabled(false);
		this->recipientFileMarkLineEdit->setEnabled(false);
		this->addressLineEdit->setEnabled(false);
		this->toHandsLineEdit->setEnabled(false);
		this->tagLineEdit->setEnabled(false);
		this->tagLineEdit->clear();
		goto finish;
	} else {
		this->subjectLineEdit->setEnabled(true);
		this->senderDbIdLineEdit->setEnabled(true);
		this->senderNameLineEdit->setEnabled(true);
		this->senderRefNumLineEdit->setEnabled(true);
		this->senderFileMarkLineEdit->setEnabled(true);
		this->recipientDbIdLineEdit->setEnabled(true);
		this->recipientNameLineEdit->setEnabled(true);
		this->recipientRefNumLineEdit->setEnabled(true);
		this->recipientFileMarkLineEdit->setEnabled(true);
		this->addressLineEdit->setEnabled(true);
		this->toHandsLineEdit->setEnabled(true);
		this->tagLineEdit->setEnabled(true);
	}

	/* search via sender databox ID */
	if (!this->senderDbIdLineEdit->text().isEmpty()) {
		this->senderDbIdLineEdit->setEnabled(true);
		this->senderNameLineEdit->setEnabled(false);
	} else if (!this->senderNameLineEdit->text().isEmpty()){
		this->senderDbIdLineEdit->setEnabled(false);
		this->senderNameLineEdit->setEnabled(true);
	} else {
		this->senderNameLineEdit->setEnabled(true);
		this->senderDbIdLineEdit->setEnabled(true);
	}

	/* search via recipient databox ID */
	if (!this->recipientDbIdLineEdit->text().isEmpty()) {
		this->recipientDbIdLineEdit->setEnabled(true);
		this->recipientNameLineEdit->setEnabled(false);
	} else if (!this->recipientNameLineEdit->text().isEmpty()){
		this->recipientDbIdLineEdit->setEnabled(false);
		this->recipientNameLineEdit->setEnabled(true);
	} else {
		this->recipientNameLineEdit->setEnabled(true);
		this->recipientDbIdLineEdit->setEnabled(true);
	}

finish:
	/* search by message ID only */
	if (!this->messageIdLineEdit->text().isEmpty()) {
		/* test if message ID is number */
		QRegExp re("\\d*");  // a digit (\d), zero or more times (*)
		/* test if message is fill and message ID > 3 chars */
		if (isAnyMsgTypeChecked &&
		    (re.exactMatch(this->messageIdLineEdit->text())) &&
		    this->messageIdLineEdit->text().size() > 2) {
			this->searchPushButton->setEnabled(true);
		} else {
			this->searchPushButton->setEnabled(false);

		}
		return;
	}

	/* search by text of tags */
	bool isTagCorrect = true;
	if (!(this->tagLineEdit->text().isEmpty()) &&
	    (this->tagLineEdit->text().length() <= 2)) {
		isTagCorrect = false;
	}

	bool isDbIdCorrect = true;
	/* databox ID must have 7 chars */
	if (!this->senderDbIdLineEdit->text().isEmpty() &&
	    this->senderDbIdLineEdit->text().size() != 7) {
			isDbIdCorrect = false;
	}
	/* databox ID must have 7 chars */
	if (!this->recipientDbIdLineEdit->text().isEmpty() &&
	    this->recipientDbIdLineEdit->text().size() != 7) {
			isDbIdCorrect = false;
	}

	/* only 3 fields can be set together */
	bool isNotFillManyFileds = true;

	int itemsWithoutTag = howManyFieldsAreFilledWithoutTag();
	if (itemsWithoutTag > 3) {
		isNotFillManyFileds = false;
		this->tooMuchFields->show();
	} else if (itemsWithoutTag < 1 &&  this->tagLineEdit->text().isEmpty()) {
		isNotFillManyFileds = false;
	}

	this->searchPushButton->setEnabled(isAnyMsgTypeChecked &&
	    isDbIdCorrect && isTagCorrect && isNotFillManyFileds);
}


/* ========================================================================= */
/*
 * Detect, how many search fileds are filled
 */
int DlgMsgSearch::howManyFieldsAreFilledWithoutTag(void)
/* ========================================================================= */
{
	int cnt = 0;

	if (!this->messageIdLineEdit->text().isEmpty()) {
		cnt++;
	}
	if (!this->subjectLineEdit->text().isEmpty()) {
		cnt++;
	}
	if (!this->senderDbIdLineEdit->text().isEmpty()) {
		cnt++;
	}
	if (!this->senderNameLineEdit->text().isEmpty())  {
		cnt++;
	}
	if (!this->addressLineEdit->text().isEmpty()) {
		cnt++;
	}
	if (!this->recipientDbIdLineEdit->text().isEmpty()) {
		cnt++;
	}
	if (!this->recipientNameLineEdit->text().isEmpty()) {
		cnt++;
	}
	if (!this->addressLineEdit->text().isEmpty()) {
		cnt++;
	}
	if (!this->senderRefNumLineEdit->text().isEmpty()) {
		cnt++;
	}
	if (!this->senderFileMarkLineEdit->text().isEmpty()) {
		cnt++;
	}
	if (!this->recipientRefNumLineEdit->text().isEmpty()) {
		cnt++;
	}
	if (!this->recipientFileMarkLineEdit->text().isEmpty()) {
		cnt++;
	}
	if (!this->toHandsLineEdit->text().isEmpty()) {
		cnt++;
	}

	return cnt;
}


/* ========================================================================= */
/*
 * Set first column with checkbox active if item was changed
 */
void DlgMsgSearch::setFirtsColumnActive(void)
/* ========================================================================= */
{
	this->resultsTableWidget->selectColumn(0);
	this->resultsTableWidget->selectRow(
	    this->resultsTableWidget->currentRow());
}


/* ========================================================================= */
/*
 * Search messages
 */
void DlgMsgSearch::searchMessages(void)
/* ========================================================================= */
{
	debugSlotCall();

	this->resultsTableWidget->setRowCount(0);
	this->resultsTableWidget->setEnabled(false);

	MessageDb::SoughtMsg msgData;

	/* holds search result data from message envelope table */
	QList<MessageDb::SoughtMsg> msgEnvlpResultList;

	/*
	 * holds all message ids of from message_tags table
	 * where input search text like with tag name
	 */
	QList<qint64> tagMsgIdList;

	/* holds messages (data) which will add to result widget */
	QList<MessageDb::SoughtMsg> msgListForTableView;

	/* message types where search process will be applied */
	enum MessageDirection msgType = MSG_ALL;
	if (this->searchReceivedMsgCheckBox->isChecked() &&
	    this->searchSentMsgCheckBox->isChecked()) {
		msgType = MSG_ALL;
	} else if (this->searchReceivedMsgCheckBox->isChecked()) {
		msgType = MSG_RECEIVED;
	} else if (this->searchSentMsgCheckBox->isChecked()) {
		msgType = MSG_SENT;
	}

	/*
	 * if tag input was filled, get message ids from message_tags table
	 * where input search text like with tag name
	*/
	bool applyTag = false;
	if (!this->tagLineEdit->text().isEmpty()) {
		tagMsgIdList = globTagDbPtr->getMsgIdsContainSearchTagText(
		    this->tagLineEdit->text());
		applyTag = true;
	}

	/* selected account or all accounts will be used for search request */
	int dbCount = 1;
	if (this->searchAllAcntCheckBox->isChecked()) {
		dbCount = m_messageDbSetList.count();
	}

	/* how many fields without tag item are filled in the search dialog */
	int itemsWithoutTag = howManyFieldsAreFilledWithoutTag();

	/* over selected account or all accounts do */
	for (int i = 0; i < dbCount; ++i) {

		msgEnvlpResultList.clear();
		msgListForTableView.clear();

		/* when at least one field is filled (without tag) */
		if (itemsWithoutTag > 0) {
			/*
			 * get messages envelope data
			 * where search items are applied
			 */
			msgEnvlpResultList = m_messageDbSetList.at(i).second->
			    msgsAdvancedSearchMessageEnvelope(
			    this->messageIdLineEdit->text().isEmpty() ? -1 :
			        this->messageIdLineEdit->text().toLongLong(),
			    this->subjectLineEdit->text(),
			    this->senderDbIdLineEdit->text(),
			    this->senderNameLineEdit->text(),
			    this->addressLineEdit->text(),
			    this->recipientDbIdLineEdit->text(),
			    this->recipientNameLineEdit->text(),
			    this->senderRefNumLineEdit->text(),
			    this->senderFileMarkLineEdit->text(),
			    this->recipientRefNumLineEdit->text(),
			    this->recipientFileMarkLineEdit->text(),
			    this->toHandsLineEdit->text(),
			    QString(), QString(), msgType);
		}

		/* Results processing section - 4 scenarios */

		/*
		 * First scenario:
		 * tag input was filled and another envelope fileds were filled,
		 * tag list and msg envelope search list results are not empty,
		 * so we must penetration both list (prunik) and
		 * choose relevant records and show it (fill msgListForView).
		 */
		if (applyTag && (!tagMsgIdList.isEmpty()) &&
		    (!msgEnvlpResultList.isEmpty())) {

			foreach (const qint64 msgId, tagMsgIdList) {
				foreach (const MessageDb::SoughtMsg msg,
				    msgEnvlpResultList) {
					if (msg.mId.dmId == msgId) {
						msgData =
						    m_messageDbSetList.at(i).
						    second->msgsGetMsgDataFromId(msgId);
						if (msgData.mId.dmId != -1) {
							msgListForTableView.append(msgData);
						}
					}
				}
			}
			if (!msgListForTableView.isEmpty()) {
				appendMsgsToTable(m_messageDbSetList.at(i),
				    msgListForTableView);
			}

		/*
		 * Second scenario:
		 * tag input was filled and another envelope fileds were filled
		 * but msg envelope search result list is empty = no match,
		 * we show (do) nothing
		 */
		} else if (applyTag && msgEnvlpResultList.isEmpty() &&
		    (itemsWithoutTag > 0)) {

		/*
		 * Third scenario:
		 * tag input was not filled and msg envelope list is not empty,
		 * we show result for msg envelope list only
		  */
		} else if (!applyTag && !msgEnvlpResultList.isEmpty()) {
			appendMsgsToTable(m_messageDbSetList.at(i),
			    msgEnvlpResultList);

		/*
		 * Last scenario:
		 * only tag input was filled and tag list are not empty,
		 * we show result for tag results only (fill msgListForView).
		 */
		} else if (applyTag && (!tagMsgIdList.isEmpty())) {

			foreach (const qint64 msgId, tagMsgIdList) {

				msgData = m_messageDbSetList.at(i).second->
				    msgsGetMsgDataFromId(msgId);

				if (msgData.mId.dmId != -1) {
					msgListForTableView.append(msgData);
				}
			}

			if (!msgListForTableView.isEmpty()) {
				appendMsgsToTable(m_messageDbSetList.at(i),
				    msgListForTableView);
			}
		}
	}
}


/* ========================================================================= */
/*
 * Append message list to result tablewidget
 */
void DlgMsgSearch::appendMsgsToTable(
    const QPair<QString, MessageDbSet *> &usrNmAndMsgDbSet,
    const QList<MessageDb::SoughtMsg> &msgDataList)
/* ========================================================================= */
{
	this->resultsTableWidget->setEnabled(true);

	foreach (const MessageDb::SoughtMsg &msgData, msgDataList) {
		int row = this->resultsTableWidget->rowCount();
		this->resultsTableWidget->insertRow(row);

		this->resultsTableWidget->setItem(row, COL_USER_NAME,
		    new QTableWidgetItem(usrNmAndMsgDbSet.first));
		QTableWidgetItem *item = new QTableWidgetItem;
		item->setText(QString::number(msgData.mId.dmId));
		if (ENABLE_TOOLTIP) {
			const MessageDb *messageDb =
			    usrNmAndMsgDbSet.second->constAccessMessageDb(
			        msgData.mId.deliveryTime);
			Q_ASSERT(0 != messageDb);

			item->setToolTip(messageDb->descriptionHtml(
			    msgData.mId.dmId, true, false, true));
		}
		this->resultsTableWidget->setItem(row, COL_MESSAGE_ID, item);
		this->resultsTableWidget->setItem(row, COL_DELIVERY_YEAR,
		    new QTableWidgetItem(
		        MessageDbSet::yearFromDateTime(
		            msgData.mId.deliveryTime)));
		this->resultsTableWidget->setItem(row, COL_MESSAGE_TYPE,
		    new QTableWidgetItem(QString::number(msgData.type)));
		this->resultsTableWidget->setItem(row, COL_ANNOTATION,
		    new QTableWidgetItem(msgData.dmAnnotation));
		this->resultsTableWidget->setItem(row, COL_SENDER,
		    new QTableWidgetItem(msgData.dmSender));
		this->resultsTableWidget->setItem(row, COL_RECIPIENT,
		    new QTableWidgetItem(msgData.dmRecipient));
	}

	this->resultsTableWidget->resizeColumnsToContents();
	this->resultsTableWidget->
	    horizontalHeader()->setStretchLastSection(true);
}


/* ========================================================================= */
/*
 * Get ID of selected message and set focus in MessageList Tableview
 */
void DlgMsgSearch::getSelectedMsg(int row, int column)
/* ========================================================================= */
{
	Q_UNUSED(column);
	emit focusSelectedMsg(
	    this->resultsTableWidget->item(row, COL_USER_NAME)->text(),
	    this->resultsTableWidget->item(row, COL_MESSAGE_ID)->text().toLongLong(),
	    this->resultsTableWidget->item(row, COL_DELIVERY_YEAR)->text(),
	    this->resultsTableWidget->item(row, COL_MESSAGE_TYPE)->text().toInt());
	//this->close();
}
