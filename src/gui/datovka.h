/*
 * Copyright (C) 2014-2015 CZ.NIC
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

#ifndef _DATOVKA_H_
#define _DATOVKA_H_


#include <QItemSelection>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QStandardItemModel>
#include <QProgressBar>
#include <QThread>
#include <QTimer>
#include <QPushButton>
#include <QNetworkReply>

#include "src/common.h"
#include "src/io/account_db.h"
#include "src/io/message_db.h"
#include "src/io/message_db_set.h"
#include "src/gui/dlg_import_zfo.h"
#include "src/gui/dlg_timestamp_expir.h"
#include "src/models/accounts_model.h"
#include "src/models/sort_filter_proxy_model.h"
#include "thread/worker.h"


namespace Ui {
	class MainWindow;
}


class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow(void);

	void setDefaultProgressStatus(void);

	QProgressBar *m_statusProgressBar; /*!< Progress-bar object. */
	QLabel *statusOnlineLabel;
	QLabel *statusDbMode;
	QStatusBar *statusBar;

	/* tmp account info struct for ZFO import */
	class AccountDataStruct {
	public:
		QString databoxID;
		QString accountName;
		QString username;
		MessageDbSet *messageDbSet;
		QModelIndex acntIndex;
	};

	enum isdsResult {
		MSG_IS_IN_ISDS,
		MSG_IS_NOT_IN_ISDS,
		MSG_ISDS_ERROR,
		MSG_FILE_ERROR
	};

	/*!
	 * @brief Create configuration file if not present.
	 */
	static
	bool ensureConfPresence(void);

	/*!
	 * @brief connect to ISDS databox from exist account
	 *
	 * @note If pointer to main windows is specified, then various
	 *     dialogues may be shown. Passwords are used only when required
	 *     (i.e. not found in configuration).
	 *
	 * @param userName Account login.
	 * @param mw       Pointer to main window.
	 * @param pwd      User password.
	 * @param otpKey   One-time password or certificate key.
	 */
	static
	bool connectToIsds(const QString &userName, MainWindow *mw,
	    const QString &pwd = QString(), const QString &otpKey = QString());

	/*!
	 * @brief Get message db set related to given account.
	 *
	 * @note If pointer to main window is not specified, then all dialogues
	 *     will be suppressed.
	 *
	 * @param userName Account login.
	 * @param mw       Pointer to main window.
	 */
	static
	MessageDbSet *accountDbSet(const QString &userName, MainWindow *mw);

	/*!
	 * @brief Get data about logged in user and his box.
	 */
	static
	bool getOwnerInfoFromLogin(const QString &userName);

	/*!
	 * @brief Get information about password expiration date.
	 */
	static
	bool getPasswordInfoFromLogin(const QString &userName);

	/*!
	 * @brief Get data about logged in user.
	 */
	static
	bool getUserInfoFromLogin(const QString &userName);

protected:
	/*!
	 * Check if some worker is working on the background and show
	 * dialog if user want to close application
	 */
	void closeEvent(QCloseEvent *event);


private slots:

	/*!
	 * @brief Clear info status bar if download of complete message fails.
	 */
	void clearInfoInStatusBarAndShowDialog(qint64 msgId,
	    const QString &errMsg);

	/*!
	 * @brief Version response slot.
	 */
	void datovkaVersionResponce(QNetworkReply* reply);

	/*!
	 * @brief Updates the account model according to the change properties.
	 */
	void updateAccountListEntry(const QString &userName);

	/*!
	 * @brief Verify whether a connection to ISDS can be establisehd
	 *     and databox exists for the account.
	 */
	void getAccountUserDataboxInfo(AccountModel::SettingsMap accountInfo);

	/*!
	 * @brief Redraws widgets according to selected account item.
	 */
	void accountItemCurrentChanged(const QModelIndex &current,
	    const QModelIndex &previous = QModelIndex());

	/*!
	 * @brief Generates menu to selected account item.
	 *     (And redraws widgets.)
	 */
	void accountItemRightClicked(const QPoint &point);

	/*!
	 * @brief Sets contents of widgets according to selected messages.
	 */
	void messageItemsSelectionChanged(const QItemSelection &selected,
	    const QItemSelection &deselected = QItemSelection());

	/*!
	 * @brief Used for toggling the message read state.
	 */
	void messageItemClicked(const QModelIndex &index);

	/*!
	 * @brief Generates menu to selected message item.
	 *     (And redraws widgets.)
	 */
	void messageItemRightClicked(const QPoint &point);

	/*!
	 * @brief Saves message selection.
	 */
	void messageItemStoreSelection(qint64 msgId);

	/*!
	 * @brief Saves message selection when model changes.
	 */
	void messageItemStoreSelectionOnModelChange(void);

	/*!
	 * @brief Restores message selection.
	 */
	void messageItemRestoreSelectionOnModelChange(void);

	/*!
	 * @brief Restores message selection.
	 */
	void messageItemRestoreSelectionAfterLayoutChange(void);

	/*!
	 * @brief Select account via userName and focus on
	 *        message ID from search selection
	 */
	void messageItemFromSearchSelection(const QString &userName,
	    qint64 msgId, const QString &deliveryYear, int msgType);

	/*!
	 * @brief Redraws widgets according to attachment item selection.
	 */
	void attachmentItemsSelectionChanged(const QItemSelection &selected,
	    const QItemSelection &deselected = QItemSelection());

	/*!
	 * @brief Generates menu to selected message item.
	 *     (And redraws widgets.)
	 */
	void attachmentItemRightClicked(const QPoint &point);

	/*!
	 * @brief Handle attachment double click.
	 */
	void attachmentItemDoubleClicked(const QModelIndex &index);

	/*!
	 * @brief Saves selected attachments to file.
	 */
	void saveSelectedAttachmentsToFile(void);

	/*!
	 * @brief Save all attachments to dir.
	 */
	void saveAllAttachmentsToDir(void);

	/*!
	 * @brief Open attachment in default application.
	 */
	void openSelectedAttachment(void);

	/*!
	 * @brief Mark all received messages in the current working account.
	 */
	void accountMarkReceivedRead(void);

	/*!
	 * @brief Mark all received messages in the current working account.
	 */
	void accountMarkReceivedUnread(void);

	/*!
	 * @brief Mark all received messages in given year in the current
	 *     working account.
	 */
	void accountMarkReceivedYearRead(void);

	/*!
	 * @brief Mark all received messages in given year in the current
	 *     working account.
	 */
	void accountMarkReceivedYearUnread(void);

	/*!
	 * @brief Mark all received messages in given year in the current
	 *     working account.
	 */
	void accountMarkRecentReceivedRead(void);

	/*!
	 * @brief Mark all received messages in given year in the current
	 *     working account.
	 */
	void accountMarkRecentReceivedUnread(void);

	/*!
	 * @brief Mark all received messages in the current working account.
	 */
	void accountMarkReceivedUnsettled(void);

	/*!
	 * @brief Mark all received messages in the current working account.
	 */
	void accountMarkReceivedInProgress(void);

	/*!
	 * @brief Mark all received messages in the current working account.
	 */
	void accountMarkReceivedSettled(void);

	/*!
	 * @brief Mark all received messages in given year in the current
	 *     working account.
	 */
	void accountMarkReceivedYearUnsettled(void);

	/*!
	 * @brief Mark all received messages in given year in the current
	 *     working account.
	 */
	void accountMarkReceivedYearInProgress(void);

	/*!
	 * @brief Mark all received messages in given year in the current
	 *     working account.
	 */
	void accountMarkReceivedYearSettled(void);

	/*!
	 * @brief Mark recently received messages in the current
	 *     working account.
	 */
	void accountMarkRecentReceivedUnsettled(void);

	/*!
	 * @brief Mark recently received messages in the current
	 *     working account.
	 */
	void accountMarkRecentReceivedInProgress(void);

	/*!
	 * @brief Mark recently received messages in the current
	 *     working account.
	 */
	void accountMarkRecentReceivedSettled(void);

	/*!
	 * @brief Mark selected messages as read.
	 */
	void messageItemsSelectedMarkRead(void);

	/*!
	 * @brief Mark selected messages as unread.
	 */
	void messageItemsSelectedMarkUnread(void);

	/*!
	 * @brief Mark selected messages as unsettled.
	 */
	void messageItemsSelectedMarkUnsettled(void);

	/*!
	 * @brief Mark selected messages as in progress.
	 */
	void messageItemsSelectedMarkInProgress(void);

	/*!
	 * @brief Mark selected messages as settled.
	 */
	void messageItemsSelectedMarkSettled(void);

	/*!
	 * @brief Delete selected message(s) from local database and ISDS.
	 */
	void deleteMessage(void);

	/*!
	 * @brief Downloads new messages from server for all accounts.
	 */
	void synchroniseAllAccounts(void);

	/*!
	 * @brief Downloads new messages from server for selected account.
	 */
	void synchroniseSelectedAccount(void);

	/*!
	 * @brief Downloads the attachments for the selected message.
	 */
	void downloadSelectedMessageAttachments(void);

	/*!
	 * @brief Process pending worker jobs.
	 */
	void processPendingWorkerJobs(void);

	/*!
	 * @brief End current worker job.
	 */
	void endCurrentWorkerJob(void);

	/*!
	 * @brief Create and send a new message form selected account.
	 */
	void createAndSendMessage(void);

	/*!
	 * @brief Create reply from the selected message.
	 */
	void createAndSendMessageReply(void);

	/*!
	 * @brief Create message from the template (selected message).
	 */
	void createAndSendMessageFromTmpl(void);

	/*!
	 * @brief Verifies selected message and creates response dialog.
	 */
	void verifySelectedMessage(void);

	/*!
	 * @brief Shows account properties dialog.
	 */
	void manageAccountProperties(void);

	/*!
	 * @brief Shows change password dialog.
	 */
	void changeAccountPassword(void);

	/*!
	 * @brief Shows the application preferences dialog.
	 */
	void applicationPreferences(void);

	/*!
	 * @brief Add account action and dialog.
	 */
	void addNewAccount(void);

	/*!
	 * @brief Deletion confirmation dialog.
	 */
	void deleteSelectedAccount(void);

	/*!
	 * @brief Show import database directory dialog.
	 */
	void showImportDatabaseDialog(void);

	/*!
	 * @brief Prepare import database directory.
	 */
	void prepareCreateAccountFromDatabaseFile(bool fromDirectory);

	/*!
	 * @brief Prepare import of messages from database.
	 */
	void prepareMsgsImportFromDatabase(void);

	/*!
	 * @brief Proxy setting dialog.
	 */
	void proxySettings(void);

	/*!
	 * @brief Move selected account up.
	 */
	void moveSelectedAccountUp(void);

	/*!
	 * @brief Move selected account down.
	 */
	void moveSelectedAccountDown(void);

	/*!
	 * @brief Change data directory dialog.
	 */
	void changeDataDirectory(void);

	/*!
	 * @brief View signature details.
	 */
	void showSignatureDetails(void);

	/*!
	 * @brief Export message into as ZFO file dialog.
	 */
	void exportSelectedMessageAsZFO(const QString &attachPath = QString(),
	    QString userName = QString(), qint64 dmID = -1,
	    const QDateTime &delivTime = QDateTime());

	/*!
	 * @brief Export delivery information as ZFO file dialog.
	 */
	void exportDeliveryInfoAsZFO(const QString &attachPath = QString(),
	    const QString &attachFileName = QString(),
	    const QString &formatString = globPref.delivery_filename_format,
	    qint64 dmID = -1, const QDateTime &delivTime = QDateTime());

	/*!
	 * @brief Export delivery information as PDF file dialog.
	 */
	void exportDeliveryInfoAsPDF(const QString &attachPath = QString(),
	    const QString &attachFileName = QString(),
	    const QString &formatString = globPref.delivery_filename_format,
	    qint64 dmID = -1, const QDateTime &delivTime = QDateTime());

	/*!
	 * @brief Export selected message envelope as PDF file dialog.
	 */
	void exportMessageEnvelopeAsPDF(const QString &attachPath = QString(),
	    qint64 dmID = -1, const QDateTime &delivTime = QDateTime());

	/*!
	 * @brief Open selected message in external application.
	 */
	void openSelectedMessageExternally(void);

	/*!
	 * @brief Open delivery information externally.
	 */
	void openDeliveryInfoExternally(void);

	/*!
	 * @brief Search data box dialog.
	 */
	void findDatabox(void);

	/*!
	 * @brief Authenticate message file dialog.
	 */
	void authenticateMessageFile(void);

	/*!
	 * @brief View message from file dialog.
	 */
	void viewMessageFromZFO(void);

	/*!
	 * @brief Export correspondence overview dialog.
	 */
	void exportCorrespondenceOverview(void);

	/*!
	 * @brief Show dialog with settings of import ZFO file(s) into database.
	 */
	void showImportZFOActionDialog(void);

	/*!
	 * @brief Create ZFO file(s) list for import into database.
	 */
	void createZFOListForImport(enum ImportZFODialog::ZFOtype zfoType,
	    enum ImportZFODialog::ZFOaction importType);

	/*!
	 * @brief Get message type of import ZFO file (message/delivery/unknown).
	 * Return: -1=error, 0=unknown, 1=message, 2=delivery info
	 */
	int getMessageTypeFromZFO(const QString &file);

	/*!
	 * @brief Create account info for ZFO file(s) import into database.
	 */
	QList<AccountDataStruct> createAccountInfoForZFOImport(void);

	/*!
	 * @brief Prepare import ZFO file(s) into database by ZFO type.
	 */
	void prepareZFOImportIntoDatabase(const QStringList &files,
	    enum ImportZFODialog::ZFOtype zfoType);

	/*!
	 * @brief Import only delivery info ZFO file(s) into database.
	 */
	void importDeliveryInfoZFO(
	    const QList<AccountDataStruct> &accountList,
	    const QStringList &files,
	    QList<QPair<QString,QString>> &successFilesList,
	    QList<QPair<QString,QString>> &existFilesList,
	    QList<QPair<QString,QString>> &errorFilesList);

	/*!
	 * @brief Import only message ZFO file(s) into database.
	 */
	void importMessageZFO(
	    const QList<AccountDataStruct> &accountList,
	    const QStringList &files,
	    QList<QPair<QString,QString>> &successFilesList,
	    QList<QPair<QString,QString>> &existFilesList,
	    QList<QPair<QString,QString>> &errorFilesList);

	/*!
	 * @brief Show ZFO import notification dialog with results of import.
	 */
	void showNotificationDialogWithResult(int filesCnt,
	    const QList<QPair<QString,QString>> &successFilesList,
	    const QList<QPair<QString,QString>> &existFilesList,
	    const QList<QPair<QString,QString>> &errorFilesList);

	/*!
	 * @brief Check if import ZFO file is/was in ISDS.
	 */
	int isImportMsgInISDS(const QString &zfoFile,
	    QModelIndex accountIndex);

	/*!
	 * @brief About application dialog.
	 */
	void aboutApplication(void);

	/*!
	 * @brief Show help/manual in a internet browser.
	 */
	void showHelp(void);

	/*!
	 * @brief Go to homepage.
	 */
	void goHome(void);

	/*!
	 * @brief Clear message filter field.
	 */
	void clearFilterField(void);

	/*!
	 * @brief Filter listed messages.
	 */
	void filterMessages(const QString &text);

	/*!
	 * @brief Set new sent/received message column widths.
	 */
	void onTableColumnResized(int index, int oldSize, int newSize);

	/*!
	 * @brief Set actual sort order for current column.
	 */
	void onTableColumnHeaderSectionClicked(int column);

	/*!
	 * @brief Set ProgressBar value and Status bar text.
	 */
	void setProgressBarFromWorker(QString label, int value);

	/*!
	 * @brief Refresh AccountList.
	 */
	void refreshAccountListFromWorker(const QString &userName);

	/*!
	 * @brief Set and run any actions after main window has been created.
	 */
	void setWindowsAfterInit(void);

	/*!
	 * @brief Receive and store new account database path. Change data
	 *     directory path in settings.
	 */
	void receiveNewDataPath(QString oldDir, QString newDir,
	    QString action);

	/*!
	 * @brief Set tablewidget when message download worker is done.
	 */
	void postDownloadSelectedMessageAttachments(const QString &userName,
	    qint64 dmId);

	/*!
	 * @brief Set info status bar from worker.
	 */
	void dataFromWorkerToStatusBarInfo(bool add,
	    int rt, int rn, int st, int sn);

	/*!
	 * @brief set message process state into db
	 */
	void msgSetSelectedMessageProcessState(int stateIndex);

	/*!
	 * @brief Show advanced message search dialogue.
	 */
	void showMsgAdvancedSearchDlg(void);

	/*!
	 * @brief On message dialogue exit.
	 */
	void msgAdvancedDlgFinished(int result);

	/*!
	 * @brief Show message timestamp expiration dialog.
	 */
	void showMsgTmstmpExpirDialog(void);

	/*!
	 * @brief Prepare message timestamp expiration based on action.
	 */
	void prepareMsgTmstmpExpir(enum TimestampExpirDialog::TSaction action);

	/*!
	 * @brief Split message database slot.
	 */
	void splitMsgDbByYearsSlot(void);

private:

	QThread *m_syncAcntThread;
	Worker *m_syncAcntWorker;
	QTimer m_timerSyncAccounts;
	int m_timeoutSyncAccounts;

	void showStatusTextWithTimeout(const QString &qStr);

	void showStatusTextPermanently(const QString &qStr);

	void saveAttachmentToFile(const QModelIndex &messageIndex,
	    const QModelIndex &attachmentIndex);

	/*!
	 * @brief Return index for yearly entry with given properties.
	 */
	QModelIndex accountYearlyIndex(const QString &userName,
	    const QString &year, int msgType);

	/*!
	 * @brief Return index for message with given properties.
	 */
	QModelIndex messageIndex(qint64 msgId) const;

	/*!
	 * @brief Check message time stamp expiration for account.
	 */
	void checkMsgsTmstmpExpiration(const QString &userName,
	    const QStringList &filePathList);

	/*!
	 * @brief Mark all received messages in the current working account.
	 */
	void accountMarkReceivedLocallyRead(bool read);

	/*!
	 * @brief Mark all received messages in given year in the current
	 *     working account.
	 */
	void accountMarkReceivedYearLocallyRead(bool read);

	/*!
	 * @brief Mark all received messages in given year in the current
	 *     working account.
	 */
	void accountMarkRecentReceivedLocallyRead(bool read);

	/*!
	 * @brief Mark all received messages in the current working account.
	 */
	void accountMarkReceivedProcessState(enum MessageProcessState state);

	/*!
	 * @brief Mark all received messages in given year in the current
	 *     working account.
	 */
	void accountMarkReceivedYearProcessState(
	    enum MessageProcessState state);

	/*!
	 * @brief Mark recently received messages in the current
	 *     working account.
	 */
	void accountMarkRecentReceivedProcessState(
	    enum MessageProcessState state);

	/*!
	 * @brief Connects top menu-bar buttons to appropriate actions.
	 */
	void connectTopMenuBarSlots(void);

	/*!
	 * @brief Connect top tool-bar buttons to appropriate actions.
	 */
	void connectTopToolBarSlots(void);

	/*!
	 * @brief Connect message-action-bar buttons to appropriate actions.
	 */
	void connectMessageActionBarSlots(void);

	/*!
	 * @brief Default settings of main window.
	 */
	void defaultUiMainWindowSettings(void) const;

	/*!
	 * @brief Set message action/button to visible.
	 */
	void setMessageActionVisibility(bool action) const;

	/*!
	 * @brief Open send message dialog and send message.
	 */
	void openSendMessageDialog(int action);

	/*!
	 * @brief Active/Inactive account menu and buttons in the mainwindow.
	 */
	void activeAccountMenuAndButtons(bool action) const;

	/*!
	 * @brief Load and apply setting from configuration file.
	 */
	void loadSettings(void);

	/*!
	 * @brief Store current setting to configuration file.
	 */
	void saveSettings(void) const;

	/*!
	 * @brief Sets geometry from settings.
	 */
	void loadWindowGeometry(const QSettings &settings);

	/*!
	 * @brief Set default account from settings.
	 */
	void setDefaultAccount(const QSettings &settings);

	/*!
	 * @brief Load sent/received messages column widths from settings.
	 */
	void loadSentReceivedMessagesColumnWidth(const QSettings &settings);

	/*!
	 * @brief Load last directory paths from settings.
	 */
	void loadLastDirectoryPaths(const QSettings &settings);

	/*!
	 * @brief Load collapse info of account items from settings.
	 */
	void loadAccountCollapseInfo(QSettings &settings);

	/*!
	 * @brief Save collapse info of account items into settings.
	 */
	void saveAccountCollapseInfo(QSettings &settings) const;

	/*!
	 * @brief Save application ID and config format
	 */
	void saveAppIdConfigFormat(QSettings &settings) const;

	/*!
	 * @brief Save sent/received messages column widths into settings.
	 */
	void saveSentReceivedColumnWidth(QSettings &settings) const;

	/*!
	 * @brief Set received message column widths.
	 */
	void setReceivedColumnWidths(void);

	/*!
	 * @brief Set sent message column widths.
	 */
	void setSentColumnWidths(void);

	/*!
	 * @brief Saves account export paths.
	 */
	void storeExportPath(void);

	/*!
	 * @brief Store geometry to settings.
	 */
	void saveWindowGeometry(QSettings &settings) const;

	/*!
	 * @brief Store current account user name to settings.
	 */
	void saveAccountIndex(QSettings &settings) const;

	/*!
	 * @brief Update numbers of unread messages in account model.
	 *
	 * param[in] index Index identifying account.
	 * @return True on success.
	 *
	 * @note Does not add/remove any nodes, just updates the counts.
	 */
	bool updateExistingAccountModelUnread(QModelIndex index);

	/*!
	 * @brief Partially regenerates account model according to the database
	 *     content.
	 *
	 * @param[in] index Index identifying account.
	 * @return True on success.
	 *
	 * @note This function adds/removes nodes and does not set the
	 *     currentIndex back to its original position.
	 */
	bool regenerateAccountModelYears(QModelIndex index);

	/*!
	 * @brief Regenerates account model according to the database content.
	 *
	 * @return True on success.
	 *
	 * @note This function add/removes nodes and does not set the
	 *     currentIndex back to its original position.
	 */
	bool regenerateAllAccountModelYears(void);

	/*!
	 * @brief Generate account info HTML message.
	 */
	QString createAccountInfo(const QString &userName);

	/*!
	 * @brief Generate overall account information.
	 */
	QString createAccountInfoAllField(const QString &accountName,
	    const QList< QPair<QString, int> > &receivedCounts,
	    const QList< QPair<QString, int> > &sentCounts) const;

	/*!
	 * @brief Generate overall account information only for sent or
	 *     received messages.
	 */
	QString createAccountInfoMessagesCount(const QString &accountName,
	    const QList< QPair<QString, int> > &counts,
	    enum MessageDb::MessageType type) const;

	/*!
	 * @brief Generate banner.
	 */
	QString createDatovkaBanner(const QString &version) const;

	/*!
	 * @brief Returns user name related to given account item.
	 *
	 * @note If accountItem is 0 then currently selected account is
	 *     processed.
	 */
	QString userNameFromItem(const QStandardItem *accountItem = 0) const;

	/*!
	 * @brief Returns pointer to account item related to given user name.
	 *
	 * @param[in] userName User login name.
	 * @return 0 when no such top item found.
	 */
	QStandardItem *itemFromUserName(const QString &userName) const;

	/*!
	 * @brief Set export paths to selected account item.
	 */
	void setAccountStoragePaths(const QString &userName);

	/*!
	 * @brief Delete message from long term storage in ISDS and
	 * local database - based on delFromIsds parameter.
	 */
	qdatovka_error eraseMessage(const QString &userName, qint64 dmId,
	    const QDateTime &deliveryTime, bool delFromIsds);

	/*!
	 * @brief Verify message. Compare hash with hash stored in ISDS.
	 */
	qdatovka_error verifyMessage(const QString &userName, qint64 dmId,
	    const QDateTime &deliveryTime);

	/*!
	 * @brief Authenticate message from ZFO file.
	 */
	qdatovka_error authenticateMessageFromZFO(void);

	/*!
	 * @brief Show message info for user if connection to ISDS fails.
	 */
	void showConnectionErrorMessageBox(int status,
	    const QString &accountName, QString isdsMsg);

	/*!
	 * @brief Check if connection to ISDS fails.
	 */
	static
	bool checkConnectionError(int status, const QString &accountName,
	    const QString &isdsMsg, MainWindow *mw);

	/*!
	 * @brief connect to ISDS databox from new account
	 */
	bool firstConnectToIsds(AccountModel::SettingsMap &accountInfo,
	    bool showDialog);

	/*!
	 * @brief Login to ISDS server by user name and password only.
	 */
	static
	bool loginMethodUserNamePwd(
	    AccountModel::SettingsMap &accountInfo,
	    MainWindow *mw, const QString &pwd = QString());

	/*!
	 * @brief Login to ISDS server by user name, password and OTP code.
	 */
	static
	bool loginMethodUserNamePwdOtp(
	    AccountModel::SettingsMap &accountInfo,
	    MainWindow *mw, const QString &pwd = QString(),
	    const QString &otp = QString());

	/*!
	 * @brief Converts PKCS #12 certificate into PEM format.
	 *
	 * @note The function creates a new PEM file stored in
	 *     the configuration directory. The path is returned via
	 *     the second parameter.
	 *
	 * @param[in]  p12Path   Path to PKCS #12 certificate file.
	 * @param[in]  certPwd   Password protecting the certificate.
	 * @param[out] pemPath   Returned path to created PEM file.
	 * @param[in]  userName  Account user name, user to name PEM file.
	 * @return True on success, false on error
	 *     (e.g. file does not exist, password error, ...)
	 */
	static
	bool p12CertificateToPem(const QString &p12Path,
	    const QString &certPwd, QString &pemPath, const QString &userName);

	/*!
	 * @brief Login to ISDS server by certificate only.
	 */
	static
	bool loginMethodCertificateOnly(AccountModel::SettingsMap &accountInfo,
	    MainWindow *mw, const QString &key = QString());

	/*!
	 * @brief Login to ISDS server by certificate, user name and password.
	 */
	static
	bool loginMethodCertificateUserPwd(
	    AccountModel::SettingsMap &accountInfo,
	    MainWindow *mw, const QString &pwd = QString(),
	    const QString &key = QString());

	/*!
	 * @brief Login to ISDS server by certificate and data box ID.
	 */
	static
	bool loginMethodCertificateIdBox(
	    AccountModel::SettingsMap &accountInfo, MainWindow *mw);

	/*!
	 * @brief Sent and check a new version of Datovka.
	 */
	void checkNewDatovkaVersion(void);

	/*!
	 * @brief Sent and check a new version of Datovka.
	 */
	void createAccountFromDatabaseFileList(
	    const QStringList &filePathList);

	/*!
	 * @brief Download complete message synchronously
	 * without worker and thread.
	 */
	bool downloadCompleteMessage(qint64 dmId,
	    const QDateTime &deliveryTime);

	/*!
	 * @brief Set read status to messages with given indexes.
	 */
	void messageItemsSetReadStatus(
	    const QModelIndexList &firstMsgColumnIdxs, bool read);

	/*!
	 * @brief Set process status to messages with given indexes.
	 */
	void messageItemsSetProcessStatus(
	    const QModelIndexList &firstMsgColumnIdxs,
	    enum MessageProcessState state);

	/*!
	 * @brief Show dialogue that notifies the user about expiring password.
	 */
	int showDialogueAboutPwdExpir(const QString &accountName,
	    const QString &userName, qint64 days, const QDateTime &dateTime);

	/*!
	 * @brief Get information about remaining PDZ credit.
	 */
	QString getPDZCreditFromISDS(void);

	/*!
	 * @brief Export message with expired time stamp to ZFO.
	 */
	void exportExpirMessagesToZFO(const QString &userName,
	    const QList<MessageDb::MsgId> &expirMsgIds);

	/*!
	 * @brief Import of messages from database to selected account.
	 */
	void doMsgsImportFromDatabase(const QStringList &dbFileList,
	    const QString &userName);

	/*!
	 * @brief Split database filename into mandatory entries.
	 *
	 * @param[in] inDbFileName - input database file name.
	 * @param[out] dbUserName - username entry.
	 * @paran[out] dbYear - year entry if exists or NULL.
	 * @paran[out] dbTestingFlag - true if account is testing or false
	 * @paran[out] errMsg - error message to user
	 * @return true if database filename is correct
	 */
	bool isValidDatabaseFileName(QString inDbFileName, QString &dbUserName,
	    QString &dbYear, bool &dbTestingFlag, QString &errMsg);

	/*!
	 * @brief Split message database into new databases
	 * contain messages for single years.
	 * @param[in] userName - username of account.
	 * @return true if success
	 */
	bool splitMsgDbByYears(const QString &userName);

	/*!
	 * @brief Show error message box
	 * @param[in] msgTitle - title of message box.
	 * @param[in] msgText - main text of message box.
	 * @param[in] msgInformativeText - info text of message box.
	 */
	void showErrMessageBox(const QString &msgTitle,
	    const QString &msgText, const QString &msgInformativeText);

	/*!
	 * @brief Set back original database path if error during
	 *    database splitting
	 * @param[in] dbset - MessageDbSet.
	 * @param[in] dbDir - origin database dir.
	 * @param[in] userName - username of account.
	 * @return true if success
	 */
	bool setBackOriginDb(MessageDbSet *dbset, const QString &dbDir,
	    const QString &userName);


	QString m_confDirName; /*!< Configuration directory location. */
	QString m_confFileName; /*!< Configuration file location. */

	AccountModel m_accountModel; /*!<
	                              * Account tree view model. Generated from
	                              * configuration file.
	                              */
	QLineEdit *m_filterLine; /*!< Search filter line object. */
	QPushButton *m_clearFilterLineButton; /*!< Button object. */
	SortFilterProxyModel m_messageListProxyModel; /*!<
	                                                * Used for message
	                                                * sorting and
	                                                * filtering.
	                                                */

	QTimer m_messageMarker; /*!< Used for marking messages as read. */
	qint64 m_lastSelectedMessageId; /*!< Id of the last selected message. */
	qint64 m_lastStoredMessageId; /*!< Last stored message selection. */
	enum AccountModel::NodeType
	    m_lastSelectedAccountNodeType; /*!< Last selected position. */
	enum AccountModel::NodeType
	    m_lastStoredAccountNodeType; /*!< Last stored account position. */

	bool m_searchDlgActive; /*!< True if search dialogue is active. */

	int m_received_1;
	int m_received_2;
	int m_sent_1;
	int m_sent_2;
	int m_sort_column;
	QString m_sort_order;
	QString m_save_attach_dir;
	QString m_add_attach_dir;
	QString m_export_correspond_dir;
	QString m_on_export_zfo_activate;
	QString m_on_import_database_dir_activate;
	QString m_import_zfo_path;
	bool isMainWindow;

	Ui::MainWindow *ui;
};


#endif /* _DATOVKA_H_ */
