

#ifndef _DLG_SEND_MESSAGE_H_
#define _DLG_SEND_MESSAGE_H_

#include <QDialog>
#include <QFileDialog>
#include <QTimer>
#include <QTreeView>

#include "src/common.h"
#include "src/io/message_db.h"
#include "ui_dlg_send_message.h"
#include "src/io/isds_sessions.h"
#include "src/models/accounts_model.h"


class DlgSendMessage : public QDialog, public Ui::SendMessage {
    Q_OBJECT

public:
	enum Action {
		ACT_NEW,
		ACT_REPLY
	};

	DlgSendMessage(MessageDb &db, QString &dbId, Action action,
	    QTreeView &accountList, QTableView &messageList,
	    const AccountModel::SettingsMap &accountInfo,
	    QString dbType, bool dbEffectiveOVM, bool dbOpenAddressing,
	    QWidget *parent = 0,
	    const QString &reSubject = QString(),
	    const QString &senderId = QString(),
	    const QString &sender = QString(),
	    const QString &senderAddress = QString(),
	    const QString &dmType = QString(),
	    const QString &dmSenderRefNumber = QString()
	    );

private slots:
	void on_cancelButton_clicked(void);
	void showOptionalForm(int);
	void showOptionalFormAndSet(int);
	void addAttachmentFile(void);
	void deleteAttachmentFile(void);
	void openAttachmentFile(void);
	void addRecipientData(void);
	void deleteRecipientData(void);
	void findRecipientData(void);
	void recItemSelect(void);
	void attItemSelect(void);
	void checkInputFields(void);
	void tableItemInsRem(void);
	void sendMessage(void);
	void pingIsdsServer(void);

private:
	QTimer *pingTimer;
	void initNewMessageDialog(void);
	QTreeView &m_accountList;
	QTableView &m_messageList;
	const QString m_dbId;
	const Action m_action;
	AccountModel::SettingsMap m_accountInfo;
	QString m_dbType;
	bool m_dbEffectiveOVM;
	bool m_dbOpenAddressing;
	QString m_reSubject;
	QString m_senderId;
	QString m_sender;
	QString m_senderAddress;
	QString m_dmType;
	QString m_dmSenderRefNumber;
	QString m_userName;
	MessageDb &m_messDb;
	int m_attachSize;

	int cmptAttachmentSize(void);
	int showErrorMessageBox(int status, QString isdsMsg);
	int showInfoAboutPDZ(int pdzCnt);
	QString getUserInfoFormIsds(QString idDbox);
};


#endif /* _DLG_SEND_MESSAGE_H_ */
