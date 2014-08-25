#include <cstddef>
#include <QMessageBox>
#include "dlg_ds_search.h"
#include "src/io/isds_sessions.h"

DlgDsSearch::DlgDsSearch(Action action, QTableWidget *recipientTableWidget,
    const AccountModel::SettingsMap &accountInfo,
    QWidget *parent, QString userName)
    : QDialog(parent),
    m_recipientTableWidget(recipientTableWidget),
    m_action(action),
    m_userName(userName),
    m_accountInfo(accountInfo)
{
	setupUi(this);
	initSearchWindow();
}


/* ========================================================================= */
/*
 * Init ISDS search dialog
 */
void DlgDsSearch::initSearchWindow(void)
/* ========================================================================= */
{
	this->dataBoxTypeCBox->addItem(tr("OVM – Orgán veřejné moci"));
	this->dataBoxTypeCBox->addItem(tr("PO – Právnická osoba"));
	this->dataBoxTypeCBox->addItem(
	    tr("PFO – Podnikající fyzická osoba"));
	this->dataBoxTypeCBox->addItem(tr("FO – Fyzická osoba"));

	this->resultsTableWidget->setColumnWidth(0,20);
	this->resultsTableWidget->setColumnWidth(1,60);
	this->resultsTableWidget->setColumnWidth(2,120);
	this->resultsTableWidget->setColumnWidth(3,100);

	connect(this->iDLineEdit, SIGNAL(textChanged(QString)),
	    this, SLOT(checkInputFields()));
	connect(this->iCLineEdit, SIGNAL(textChanged(QString)),
	    this, SLOT(checkInputFields()));
	connect(this->nameLineEdit, SIGNAL(textChanged(QString)),
	    this, SLOT(checkInputFields()));
	connect(this->pscLineEdit, SIGNAL(textChanged(QString)),
	    this, SLOT(checkInputFields()));
	connect(this->dataBoxTypeCBox, SIGNAL(currentIndexChanged (int)),
	    this, SLOT(checkInputFields()));
	connect(this->resultsTableWidget,SIGNAL(itemClicked(QTableWidgetItem*)),
	    this, SLOT(enableOkButton()));
	connect(this->buttonBox, SIGNAL(accepted()), this,
	    SLOT(insertDsItems()));
	connect(this->searchPushButton, SIGNAL(clicked()), this,
	    SLOT(searchDataBox()));

	this->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	this->resultsTableWidget->
	    setEditTriggers(QAbstractItemView::NoEditTriggers);

}


/* ========================================================================= */
/*
 *  Checke input fields in the dialog
 */
void DlgDsSearch::checkInputFields(void)
/* ========================================================================= */
{
	if (this->dataBoxTypeCBox->currentIndex() == 3) {
		this->iCLineEdit->setEnabled(false);
	} else {
		this->iCLineEdit->setEnabled(true);
	}

	if (!this->iDLineEdit->text().isEmpty()) {
		this->nameLineEdit->setEnabled(false);
		this->pscLineEdit->setEnabled(false);

		if (this->iDLineEdit->text().length() == 7) {
			this->searchPushButton->setEnabled(true);
		}
		else {
			this->searchPushButton->setEnabled(false);
		}
	} else if (!this->iCLineEdit->text().isEmpty()) {
		this->iDLineEdit->setEnabled(false);
		this->nameLineEdit->setEnabled(false);
		this->pscLineEdit->setEnabled(false);

		if (this->iCLineEdit->text().length() == 8) {
			this->searchPushButton->setEnabled(true);
		}
		else {
			this->searchPushButton->setEnabled(false);
		}
	} else if (!this->nameLineEdit->text().isEmpty()) {
		this->iDLineEdit->setEnabled(false);

		if (this->nameLineEdit->text().length() > 2) {
			this->searchPushButton->setEnabled(true);
		}
		else {
			this->searchPushButton->setEnabled(false);
		}
	} else {
		this->searchPushButton->setEnabled(false);
		this->iDLineEdit->setEnabled(true);
		this->nameLineEdit->setEnabled(true);
		this->pscLineEdit->setEnabled(true);
	}
}


/* ========================================================================= */
/*
 *  Call ISDS and find databoxes via given criteria
 */
void DlgDsSearch::searchDataBox(void)
/* ========================================================================= */
{
	this->resultsTableWidget->setRowCount(0);
	this->resultsTableWidget->setEnabled(false);

	struct isds_PersonName *personName = NULL;
	struct isds_Address *address = NULL;
	struct isds_BirthInfo *birthInfo = NULL;
	QList<QVector<QString>> list_contacts;

	isds_DbType dbType;
	int index = this->dataBoxTypeCBox->currentIndex();

	if (index == 0) {
		dbType = DBTYPE_OVM;
	} else if (index == 1) {
		dbType = DBTYPE_PO;
	} else if (index == 2) {
		dbType = DBTYPE_PFO;
	} else {
		dbType = DBTYPE_FO;
	}

	personName = isds_PersonName_add(this->nameLineEdit->text(),
	    this->nameLineEdit->text(), this->nameLineEdit->text(),
	    this->nameLineEdit->text());
	address = isds_Address_add("","","","", this->pscLineEdit->text(), "");

	struct isds_list *boxes = NULL;

	if (!isdsSessions.isConnectToIsds(m_accountInfo.userName())) {
		isdsSessions.connectToIsds(m_accountInfo);
	}


	isds_DbOwnerInfo_search(&boxes, m_userName,
	    this->iDLineEdit->text(), dbType,
	    this->iCLineEdit->text(), personName, this->nameLineEdit->text(),
	    birthInfo, address, "", "", "", "", "", 1, false, false);

	struct isds_list *box;
	box = boxes;

	if (0 == box) {
		QMessageBox::information(this, tr("Search result"),
		tr("Sorry, item(s) not found..."), QMessageBox::Ok);
	} else {
		while (0 != box) {
			this->resultsTableWidget->setEnabled(true);
			isds_DbOwnerInfo *item = (isds_DbOwnerInfo *) box->data;
			Q_ASSERT(0 != item);
			qDebug() << item->dbID;
			QVector<QString> contact;
			contact.append(item->dbID);
			contact.append(item->firmName);
			contact.append(QString(item->address->adStreet) +
			    " " + QString(item->address->adNumberInStreet));
			contact.append(QString(item->address->adZipCode));
			list_contacts.append(contact);
			addContactsToTable(list_contacts);

			box = box->next;
		}
		this->resultsTableWidget->resizeColumnsToContents();
	}

	isds_PersonName_free(&personName);
	isds_Address_free(&address);
	isds_BirthInfo_free(&birthInfo);
	isds_list_free(&boxes);
}


/* ========================================================================= */
/*
 *  Enable action button
 */
void DlgDsSearch::enableOkButton(void)
/* ========================================================================= */
{
	this->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	for (int i = 0; i < this->resultsTableWidget->rowCount(); i++) {
		if (this->resultsTableWidget->item(i,0)->checkState()) {
			this->buttonBox->button(QDialogButtonBox::Ok)->
			    setEnabled(true);
		}
	}
}


/* ========================================================================= */
/*
 * At contact list from ISDS into search result table widget
 */
void DlgDsSearch::addContactsToTable(
    const QList< QVector<QString> > &contactList)
/* ========================================================================= */
{
	this->resultsTableWidget->setRowCount(0);

	this->resultsTableWidget->setEnabled(true);
	for (int i = 0; i < contactList.count(); i++) {

		int row = this->resultsTableWidget->rowCount();
		this->resultsTableWidget->insertRow(row);
		QTableWidgetItem *item = new QTableWidgetItem;
		item->setCheckState(Qt::Unchecked);
		this->resultsTableWidget->setItem(row,0,item);
		item = new QTableWidgetItem;
		item->setText(contactList[i].at(0));
		this->resultsTableWidget->setItem(row,1,item);
		item = new QTableWidgetItem;
		item->setText(contactList[i].at(1));
		this->resultsTableWidget->setItem(row,2,item);
		item = new QTableWidgetItem;
		item->setText(contactList[i].at(2));
		this->resultsTableWidget->setItem(row,3,item);
		item = new QTableWidgetItem;
		item->setText(contactList[i].at(3));
		this->resultsTableWidget->setItem(row,4,item);
	}
}


/* ========================================================================= */
/*
 *  Test if the selected item is not in recipient list
 */
bool DlgDsSearch::isInRecipientTable(const QString &idDs) const
/* ========================================================================= */
{
	Q_ASSERT(0 != m_recipientTableWidget);

	for (int i = 0; i < this->m_recipientTableWidget->rowCount(); i++) {
		if (this->m_recipientTableWidget->item(i,0)->text() == idDs) {
			return true;
		}
	}
	return false;
}


/* ========================================================================= */
/*
 *  Insert selected item into recipient list of the sent message dialog
 */
void DlgDsSearch::insertDsItems(void)
/* ========================================================================= */
{
	if (ACT_ADDNEW == m_action) {
		Q_ASSERT(0 != m_recipientTableWidget);
		for (int i = 0; i < this->resultsTableWidget->rowCount(); i++) {
			if ((!this->resultsTableWidget->item(
			          i,0)->checkState()) ||
			    isInRecipientTable(this->resultsTableWidget->item(
			        i,1)->text())) {
				continue;
			}
			/* For all checked non-duplicated. */
			int row = m_recipientTableWidget->rowCount();
			m_recipientTableWidget->insertRow(row);
			QTableWidgetItem *item = new QTableWidgetItem;
			item->setText(this->resultsTableWidget->
			    item(i,1)->text());
			this->m_recipientTableWidget->setItem(row,0,item);
			item = new QTableWidgetItem;
			item->setText(this->resultsTableWidget->
			    item(i,2)->text());
			this->m_recipientTableWidget->setItem(row,1,item);
			item = new QTableWidgetItem;
			item->setText(this->resultsTableWidget->\
			    item(i,3)->text());
			this->m_recipientTableWidget->setItem(row,2,item);
		}
	}
}
