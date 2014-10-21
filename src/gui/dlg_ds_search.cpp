

#include <cstddef>
#include <QMessageBox>

#include "dlg_ds_search.h"
#include "src/io/isds_sessions.h"


DlgDsSearch::DlgDsSearch(Action action, QTableWidget *recipientTableWidget,
    QString dbType, bool dbEffectiveOVM, bool dbOpenAddressing,
    QWidget *parent, QString userName)
    : QDialog(parent),
    m_action(action),
    m_recipientTableWidget(recipientTableWidget),
    m_dbType(dbType),
    m_dbEffectiveOVM(dbEffectiveOVM),
    m_dbOpenAddressing(dbOpenAddressing),
    m_userName(userName),
    m_showInfoLabel(false)
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
	QString dbOpenAddressing = "";
	QString toolTipInfo = "";

	if ("OVM" != m_dbType && !m_dbEffectiveOVM) {
		if (m_dbOpenAddressing) {
			toolTipInfo = tr("Your account is of type")
			    + " " + m_dbType + ".\n" +
			    tr("You have also Post Data Messages activated.\n"
			    "This means you can only search for accounts of "
			    "type OVM and accounts that have Post Data "
			    "Messages delivery activated.\nBecause of this "
			    "limitation the results of your current search "
			    "might not contain all otherwise matching"
			    " databoxes.");
			dbOpenAddressing = " - " +
			    tr("commercial messages are enabled");
		} else {
			toolTipInfo = tr("Your account is of type")
			    + " " + m_dbType + ".\n" +
			    tr("This means you can only search for accounts of "
			    "type OVM.\nThe current search settings will thus "
			    "probably yield no result.");
			dbOpenAddressing = " - " +
			    tr("commercial messages are disabled");
		}
		m_showInfoLabel = true;
	}

	this->accountInfo->setText("<strong>" + m_userName +
	    "</strong>" + " - " + m_dbType + dbOpenAddressing);

	this->infoLabel->setStyleSheet("QLabel { color: red }");
	this->infoLabel->setToolTip(toolTipInfo);
	this->infoLabel->hide();

	this->dataBoxTypeCBox->addItem(tr("OVM – Orgán veřejné moci"));
	this->dataBoxTypeCBox->addItem(tr("PO – Právnická osoba"));
	this->dataBoxTypeCBox->addItem(tr("PFO – Podnikající fyzická osoba"));
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

	pingTimer = new QTimer(this);
	pingTimer->start(DLG_ISDS_KEEPALIVE_MS);

	connect(pingTimer, SIGNAL(timeout()), this,
	    SLOT(pingIsdsServer()));



	checkInputFields();
}


/* ========================================================================= */
/*
 * Ping isds server, test if connection on isds server is active
 */
void DlgDsSearch::pingIsdsServer(void)
/* ========================================================================= */

{
	if (isdsSessions.isConnectToIsds(m_userName)) {
		qDebug() << "Connection to ISDS is alive :)";
	} else {
		qDebug() << "Connection to ISDS is dead :(";
	}
}

/* ========================================================================= */
/*
 *  Checke input fields in the dialog
 */
void DlgDsSearch::checkInputFields(void)
/* ========================================================================= */
{

	switch (this->dataBoxTypeCBox->currentIndex()) {
	/* OVM */
	case 0:
		this->iCLineEdit->setEnabled(true);
		this->labelName->setText(tr("Subject Name:"));
		this->labelName->setToolTip(tr("Enter name of subject"));
		this->nameLineEdit->setToolTip(tr("Enter name of subject"));
		this->infoLabel->hide();
		break;
	/* PO */
	case 1:
		this->iCLineEdit->setEnabled(true);
		this->labelName->setText(tr("Subject Name:"));
		this->labelName->setToolTip(tr("Enter name of subject"));
		this->nameLineEdit->setToolTip(tr("Enter name of subject"));
		if (m_showInfoLabel) {
			this->infoLabel->show();
		} else {
			this->infoLabel->hide();
		}
		break;
	/* FPO */
	case 2:
		this->iCLineEdit->setEnabled(true);
		this->labelName->setText(tr("Name:"));
		this->labelName->setToolTip(tr("Enter PFO last name or "
		    "firm name."));
		this->nameLineEdit->setToolTip(tr("Enter PFO last name or "
		    "firm name."));
		if (m_showInfoLabel) {
			this->infoLabel->show();
		} else {
			this->infoLabel->hide();
		}
		break;
	/* FO */
	case 3:
		this->iCLineEdit->setEnabled(false);
		this->labelName->setText(tr("Last Name:"));
		this->labelName->setToolTip(tr("Enter last name or "
		    "birth last name of FO."));
		this->nameLineEdit->setToolTip(tr("Enter last name or "
		    "birth last name of FO."));
		if (m_showInfoLabel) {
			this->infoLabel->show();
		} else {
			this->infoLabel->hide();
		}
		break;
	default:
		break;
	}

	if (!this->iDLineEdit->text().isEmpty()) {
		this->nameLineEdit->setEnabled(false);
		this->pscLineEdit->setEnabled(false);
		this->iCLineEdit->setEnabled(false);

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
	struct isds_list *boxes = NULL;
	QList<QVector<QString>> list_contacts;

	isds_DbType dbType;

	switch (this->dataBoxTypeCBox->currentIndex()) {
	case 3:
		dbType = DBTYPE_FO;
		break;
	case 2:
		dbType = DBTYPE_PFO;
		break;
	case 1:
		dbType = DBTYPE_PO;
		break;
	default:
		dbType = DBTYPE_OVM;
		break;
	}

	personName = isds_PersonName_add("", "",
	    this->nameLineEdit->text(), this->nameLineEdit->text());
	address = isds_Address_add("","","","", this->pscLineEdit->text(), "");

	isds_error status;

	status = isds_DbOwnerInfo_search(&boxes, m_userName,
	    this->iDLineEdit->text(), dbType,
	    this->iCLineEdit->text(), personName, this->nameLineEdit->text(),
	    NULL, address, "", "", "", "", "", 0, false, false);

	switch (status) {
	case IE_SUCCESS:
		break;
	case IE_NOEXIST:
		QMessageBox::information(this, tr("Search result"),
		    tr("Sorry, item(s) not found.<br><br>Try again..."),
		    QMessageBox::Ok);
		goto exit;
		break;
	case IE_ISDS:
		QMessageBox::information(this, tr("Search result"),
		    tr("Ambiguous lookup values.<br><br>Try again..."),
		    QMessageBox::Ok);
		goto exit;
		break;
	default:
		QMessageBox::critical(this, tr("Search error"),
		    tr("It is not possible find databox, because error..."),
		    QMessageBox::Ok);
		goto exit;
		break;
	}

	struct isds_list *box;
	box = boxes;

	while (0 != box) {

		this->resultsTableWidget->setEnabled(true);
		isds_DbOwnerInfo *item = (isds_DbOwnerInfo *) box->data;
		Q_ASSERT(0 != item);
		qDebug() << item->dbID;
		QVector<QString> contact;
		contact.append(item->dbID);

		QString name;

		if (*item->dbType == DBTYPE_FO) {
			name = QString(item->personName->pnFirstName) +
			    " " + QString(item->personName->pnLastName);
		} else if (*item->dbType == DBTYPE_PFO) {
			QString firmName = item->firmName;
			if (firmName.isEmpty() || firmName == " ") {
				name = QString(item->personName->pnFirstName) +
				    " " + QString(item->personName->pnLastName);
			} else {
				name = item->firmName;
			}
		} else {
			name = item->firmName;
		}

		QString address;
		QString street = item->address->adStreet;
		QString adNumberInStreet = item->address->adNumberInStreet;
		QString adNumberInMunicipality =
		    item->address->adNumberInMunicipality;

		if (street.isEmpty() || street == " ") {
			address = item->address->adCity;
		} else {
			address = street;
			if (adNumberInStreet.isEmpty() ||
			    adNumberInStreet == " ") {
				address += + " " +
				    adNumberInMunicipality +
				    ", " + QString(item->address->adCity);
			} else if (adNumberInMunicipality.isEmpty() ||
			    adNumberInMunicipality == " ") {
				address += + " " +
				    adNumberInStreet +
				    ", " + QString(item->address->adCity);
			} else {
				address += + " "+
				    adNumberInMunicipality
				    + "/" +
				    adNumberInStreet +
				", " + QString(item->address->adCity);
			}
		}

		contact.append(name);
		contact.append(address);
		contact.append(QString(item->address->adZipCode));

		list_contacts.append(contact);
		addContactsToTable(list_contacts);

		box = box->next;
	}

	this->resultsTableWidget->resizeColumnsToContents();

exit:
	isds_PersonName_free(&personName);
	isds_Address_free(&address);
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
