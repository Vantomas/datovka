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

#ifndef _MESSAGE_DB_H_
#define _MESSAGE_DB_H_

#include <QDateTime>
#include <QJsonDocument>
#include <QList>
#include <QObject>
#include <QPair>
#include <QStringList>
#include <QString>
#include <QVector>

#include "src/common.h"
#include "src/io/sqlite/db.h"

#define INVALID_YEAR "inv"
#define DB2 "db2"

enum Sorting {
	UNSORTED = 0,
	ASCENDING,
	DESCENDING
};

/*!
 * @brief Encapsulates message database.
 */
class MessageDb : public SQLiteDb {

public:
	/*!
	 * @brief Used to distinguish between sent and received messages in db.
	 *
	 * @note This value cannot be changed without breaking backward
	 *     compatibility.
	 */
	enum MessageType {
		TYPE_RECEIVED = 1, /*!< One is received. */
		TYPE_SENT = 2 /*!< Two is sent. */
	};

	/*!
	 * @brief Message identifier.
	 *
	 * @note Messages are identified according to their id and delivery
	 *     time.
	 */
	class MsgId {
	public:
		qint64 dmId; /*!< Message identifier. */
		QDateTime deliveryTime; /*!< Message delivery time. */

		MsgId(void) : dmId(-1), deliveryTime()
		{ }
		MsgId(const MsgId &id)
		    : dmId(id.dmId), deliveryTime(id.deliveryTime)
		{ }
		MsgId(qint64 id, const QDateTime &dTime)
		    : dmId(id), deliveryTime(dTime)
		{ }
		~MsgId(void)
		{ }

		MsgId &operator=(const MsgId &other) Q_DECL_NOTHROW
		{
			dmId = other.dmId;
			deliveryTime = other.deliveryTime;
			return *this;
		}

#ifdef Q_COMPILER_RVALUE_REFS
		inline
		MsgId &operator=(MsgId &&other) Q_DECL_NOTHROW
		{
			/* qSwap() is obsolete */
			std::swap(dmId, other.dmId);
			std::swap(deliveryTime, other.deliveryTime);
			return *this;
		}
#endif /* Q_COMPILER_RVALUE_REFS */

		bool isValid(void) const
		{
			return (dmId >= 0) && (deliveryTime.isValid());
		}
	};

	/*!
	 * @brief Basic message information for search dialogue.
	 */
	class SoughtMsg {
	public:
		MsgId mId; /*!< Message identifier. */
		int type; /*!< Matches enum MessageType. */
		QString dmAnnotation; /*!< Message annotation. */
		QString dmSender; /*!< Message sender. */
		QString dmRecipient; /*!< Recipient. */

		SoughtMsg(void)
		    : mId(), type(0), dmAnnotation(), dmSender(), dmRecipient()
		{ }
		SoughtMsg(const SoughtMsg &msg)
		    : mId(msg.mId), type(msg.type),
		    dmAnnotation(msg.dmAnnotation), dmSender(msg.dmSender),
		    dmRecipient(msg.dmRecipient)
		{ }
		SoughtMsg(const MsgId &id, int t, const QString &annot,
		    const QString &sen, const QString &rec)
		    : mId(id), type(t), dmAnnotation(annot),
		    dmSender(sen), dmRecipient(rec)
		{ }
		SoughtMsg(qint64 id, const QDateTime &dTime, int t,
		    const QString &annot, const QString &sen,
		    const QString &rec)
		    : mId(id, dTime), type(t), dmAnnotation(annot),
		    dmSender(sen), dmRecipient(rec)
		{ }
		~SoughtMsg(void)
		{ }

		SoughtMsg &operator=(const SoughtMsg &other) Q_DECL_NOTHROW
		{
			mId = other.mId;
			type = other.type;
			dmAnnotation = other.dmAnnotation;
			dmSender = other.dmSender;
			dmRecipient = other.dmRecipient;
			return *this;
		}

#ifdef Q_COMPILER_RVALUE_REFS
		inline
		SoughtMsg &operator=(SoughtMsg &&other) Q_DECL_NOTHROW
		{
			/* qSwap() is obsolete */
			std::swap(mId, other.mId);
			std::swap(type, other.type);
			std::swap(dmAnnotation, other.dmAnnotation);
			std::swap(dmSender, other.dmSender);
			std::swap(dmRecipient, other.dmRecipient);
			return *this;
		}
#endif /* Q_COMPILER_RVALUE_REFS */

		bool isValid(void) const
		{
			return mId.isValid() &&
			    ((type == TYPE_RECEIVED) || (type == TYPE_SENT)) &&
			    (!dmAnnotation.isEmpty()) &&
			    (!dmSender.isEmpty()) && (!dmRecipient.isEmpty());
		}
	};

	/*!
	 * @brief Received entries.
	 */
	class RcvdEntry {
	public:
		qint64 dmId; /*!< Message identifier. */
		QString dmAnnotation; /*!< Message annotation. */
		QString dmSender; /*!< Message sender. */
		QString dmDeliveryTime; /*!< Delivery time as stored in the database. */
		QString dmAcceptanceTime; /*!< Acceptance time as stored in the database. */
		bool readLocally; /*!< True if locally read. */
		bool isDownloaded; /*!< True if complete message has been downloaded. */
		int processStatus; /*!< Brief processing status. */

		RcvdEntry(qint64 i, const QString &a, const QString &s,
		    const QString &dt, const QString &at, bool rl, bool id,
		    int ps)
		    : dmId(i), dmAnnotation(a), dmSender(s), dmDeliveryTime(dt),
		    dmAcceptanceTime(at), readLocally(rl), isDownloaded(id),
		    processStatus(ps)
		{ }
	};

	/*!
	 * @brief Sent entries.
	 */
	class SntEntry {
	public:
		qint64 dmId; /*!< Message identifier. */
		QString dmAnnotation; /*!< Message annotation. */
		QString dmRecipient; /*!< Message recipient. */
		QString dmDeliveryTime; /*!< Delivery time as stored in the database. */
		QString dmAcceptanceTime; /*!< Acceptance time as stored in the database. */
		int dmMessageStatus; /*!< Brief message status. */
		bool isDownloaded; /*!< True if complete message has been downloaded. */

		SntEntry(qint64 i, const QString &a, const QString &r,
		    const QString &dt, const QString &at, int ms, bool id)
		    : dmId(i), dmAnnotation(a), dmRecipient(r),
		    dmDeliveryTime(dt), dmAcceptanceTime(at),
		    dmMessageStatus(ms), isDownloaded(id)
		{ }
	};

	/*!
	 * @brief File entry data.
	 */
	class FileData {
	public:
		QString dmFileDescr; /*!< File name. */
		QByteArray dmEncodedContent; /*!< Base64-encoded file content. */

		FileData(void)
		    : dmFileDescr(), dmEncodedContent()
		{ }
		FileData(const QString &fileDescr,
		    const QByteArray &encodedContent)
		    :  dmFileDescr(fileDescr), dmEncodedContent(encodedContent)
		{ }
		~FileData(void)
		{ }

		bool isValid(void) const
		{
			return (!dmFileDescr.isEmpty()) &&
			    (!dmEncodedContent.isEmpty());
		}
	};

	/*!
	 * @brief Attachment data used to fill attachment model.
	 */
	class AttachmentEntry {
	public:
		qint64 id; /*!< Entry identifier. */
		qint64 messageId; /*!< Identifier of the message which the attachment belong to. */
		QByteArray dmEncodedContent; /*!< Base64-encoded file content. */
		QString dmFileDescr; /*!< Attachment file name. */
		QString dmMimeType; /*!< String holding the mime type. */
		int size; /*!< Attachment file size (base64-decoded). */

		AttachmentEntry(void)
		    : id(0), messageId(0), dmEncodedContent(), dmFileDescr(),
		    dmMimeType(), size(0)
		{ }
		AttachmentEntry(qint64 i, qint64 mi, const QByteArray &dec,
		    const QString &dfd, const QString &dmt, int s)
		    : id(i), messageId(mi), dmEncodedContent(dec),
		    dmFileDescr(dfd), dmMimeType(dmt), size(s)
		{ }

		bool isValid(void) const
		{
			return (!dmFileDescr.isEmpty()) &&
			    (!dmEncodedContent.isEmpty());
		}
	};

	class PartialEnvelopeData {
	public:
		QString dbIDSender;
		QString dmSender;
		QString dmSenderAddress;
		QString dmSenderType;
		QString dbIDRecipient;
		QString dmRecipient;
		QString dmRecipientAddress;
		QString dmAnnotation;
		QString dmSenderRefNumber;
		QString dmSenderIdent;
		QString dmRecipientRefNumber;
		QString dmRecipientIdent;
		QString dmToHands;
		bool dmPersonalDelivery;
		bool dmAllowSubstDelivery;
		QString dmLegalTitleLaw;
		QString dmLegalTitleYear;
		QString dmLegalTitleSect;
		QString dmLegalTitlePar;
		QString dmLegalTitlePoint;
		QString dmType;
	};

	class ContactEntry {
	public:
		qint64 dmId; /*!< Message id. */
		QString boxId;
		QString name;
		QString address;
	};


	/* holds some additional entries for filename creation */
	class FilenameEntry {
	public:
		QDateTime dmDeliveryTime;
		QDateTime dmAcceptanceTime;
		QString dmAnnotation;
		QString dmSender;
	};

	class MessageHash {
	public:
		QByteArray valueBase64; /*!< Base-64 encoded hash value. */
		QString alg; /*!< Algorithm identifier. */

		MessageHash(void)
		    : valueBase64(), alg()
		{ }

		MessageHash(const QByteArray &b64, const QString &a)
		    : valueBase64(b64), alg(a)
		{ }

		bool isValid(void)
		{
			return (!valueBase64.isEmpty()) && (!alg.isEmpty());
		}
	};

	/*!
	 * @brief Constructor.
	 *
	 * @param[in] connectionName Connection name.
	 */
	explicit MessageDb(const QString &connectionName);

	/*!
	 * @brief Generate information for reply dialogue.
	 *
	 * @param[in] dmId  Message id.
	 * @return Vector containing title, senderId, sender, senderAddress,
	 *     mesageType, senderRefNumber.
	 *     Returns empty vector in failure.
	 */
	PartialEnvelopeData msgsReplyData(qint64 dmId) const;

	/*!
	 * @brief Return message type (sent or received).
	 *
	 * @param[in] dmId Message id.
	 * @return Message type value, negative value on error.
	 */
	int msgMessageType(qint64 dmId) const;

	/*!
	 * @brief Returns true if verification attempt was performed.
	 *
	 * @param[in] dmId  Message id.
	 * @return True is message has been verified. False may be returned
	 *     also on error.
	 */
	bool msgsVerificationAttempted(qint64 dmId) const;

	/*!
	 * @brief Returns whether message is verified.
	 *
	 * @param[in] dmId  Message identifier.
	 * @return True if message was verified successfully. False may be
	 *     returned also on error.
	 */
	bool msgsVerified(qint64 dmId) const;

	/*!
	 * @brief Returns whether message was read locally.
	 *
	 * @param[in] dmId  Message id.
	 * @retunrn False if not read or on failure.
	 */
	bool smsgdtLocallyRead(qint64 dmId) const;

	/*!
	 * @brief Set message read locally status.
	 *
	 * @param[in] dmId  Message id.
	 * @param[in] read  New read status.
	 * @return True on success.
	 */
	bool smsgdtSetLocallyRead(qint64 dmId, bool read = true);

	/*!
	 * @brief Return HTML formatted message description.
	 *
	 * @param[in]     dmId Message identifier.
	 * @param[in]     showId Whether to also show the message id.
	 * @param[in]     verSignature Whether to show verification details.
	 * @param[in]     warnOld
	 * @return HTML formatted string containing message information.
	 *     Empty string is returned on error.
	 */
	QString descriptionHtml(qint64 dmId, bool showId = true,
	    bool verSignature = true, bool warnOld = true) const;

	/*!
	 * @brief Return message envelope HTML to be used to generate a PDF.
	 *
	 * @param[in] dmId    Message identifier.
	 * @param[in] dbType  Data-box type string.
	 * @return HTML formatted string generated from message envelope.
	 *     Empty string is returned on error.
	 */
	QString envelopeInfoHtmlToPdf(qint64 dmId,
	    const QString &dbType) const;

	/*!
	 * @brief Return message file list HTML to be used to generate a PDF.
	 *
	 * @param[in] dmId    Message identifier.
	 * @return HTML formatted string generated from message file db.
	 *     Empty string is returned on error.
	 */
	QString fileListHtmlToPdf(qint64 dmId) const;

	/*!
	 * @brief Return message delivery info HTML to be used to generate
	 *     a PDF.
	 *
	 * @paramp[in] dmId  Message identifier.
	 * @return HTML formatted string generated from message delivery
	 *     information. Empty string is returned on error.
	 */
	QString deliveryInfoHtmlToPdf(qint64 dmId) const;

	/*!
	 * @brief Return fileList related to given message.
	 *
	 * @param[in] msgId  Message identifier.
	 * @return List of files and their attributes.
	 */
	QList<FileData> getFilesFromMessage(qint64 msgId) const;

	/*!
	 * @brief Return list of attachment entries related to given message.
	 *
	 * @param[in] msgId  Message identifier.
	 * @return List of attachment entries.
	 */
	QList<AttachmentEntry> attachEntries(qint64 msgId) const;

	/*!
	 * @brief Check if any message with given id exists in database.
	 *
	 * @param[in] dmId  Message identifier.
	 * @return Message status if message exists, on error or if message
	 *     does not exist in database.
	 */
	int msgsStatusIfExists(qint64 dmId) const;

	/*!
	 * @brief Check if delivery info exists in the table.
	 *
	 * @param[in] dmId  Message identifier.
	 * @return True if delivery information exist in database.
	 *     Fail is also returned on error.
	 */
	bool isDeliveryInfoRawDb(qint64 dmId) const;

	/*!
	 * @brief Insert newly sent message into messages table.
	 *
	 * @return True on success.
	 */
	bool msgsInsertNewlySentMessageEnvelope(qint64 dmId,
	    const QString &dbIDSender, const QString &dmSender,
	    const QString &dbIDRecipient, const QString &dmRecipient,
	    const QString &dmRecipientAddress, const QString &dmAnnotation);

	/*!
	 * @brief Insert message envelope into messages table.
	 *
	 * @return True on success.
	 */
	bool msgsInsertMessageEnvelope(qint64 dmId,
	    const QString &_origin, const QString &dbIDSender,
	    const QString &dmSender, const QString &dmSenderAddress,
	    int dmSenderType, const QString &dmRecipient,
	    const QString &dmRecipientAddress,
	    const QString &dmAmbiguousRecipient,
	    const QString &dmSenderOrgUnit, const QString &dmSenderOrgUnitNum,
	    const QString &dbIDRecipient, const QString &dmRecipientOrgUnit,
	    const QString &dmRecipientOrgUnitNum, const QString &dmToHands,
	    const QString &dmAnnotation, const QString &dmRecipientRefNumber,
	    const QString &dmSenderRefNumber, const QString &dmRecipientIdent,
	    const QString &dmSenderIdent, const QString &dmLegalTitleLaw,
	    const QString &dmLegalTitleYear, const QString &dmLegalTitleSect,
	    const QString &dmLegalTitlePar, const QString &dmLegalTitlePoint,
	    bool dmPersonalDelivery, bool dmAllowSubstDelivery,
	    const QByteArray &dmQTimestampBase64,
	    const QString &dmDeliveryTime, const QString &dmAcceptanceTime,
	    int dmMessageStatus, int dmAttachmentSize, const QString &_dmType,
	    enum MessageDirection msgDirect);

	/*!
	 * @brief Update message envelope into messages table.
	 *
	 * @return True on success.
	 */
	bool msgsUpdateMessageEnvelope(qint64 dmId,
	    const QString &_origin, const QString &dbIDSender,
	    const QString &dmSender, const QString &dmSenderAddress,
	    int dmSenderType, const QString &dmRecipient,
	    const QString &dmRecipientAddress,
	    const QString &dmAmbiguousRecipient,
	    const QString &dmSenderOrgUnit, const QString &dmSenderOrgUnitNum,
	    const QString &dbIDRecipient, const QString &dmRecipientOrgUnit,
	    const QString &dmRecipientOrgUnitNum, const QString &dmToHands,
	    const QString &dmAnnotation, const QString &dmRecipientRefNumber,
	    const QString &dmSenderRefNumber, const QString &dmRecipientIdent,
	    const QString &dmSenderIdent, const QString &dmLegalTitleLaw,
	    const QString &dmLegalTitleYear, const QString &dmLegalTitleSect,
	    const QString &dmLegalTitlePar, const QString &dmLegalTitlePoint,
	    bool dmPersonalDelivery, bool dmAllowSubstDelivery,
	    const QByteArray &dmQTimestampBase64,
	    const QString &dmDeliveryTime, const QString &dmAcceptanceTime,
	    int dmMessageStatus, int dmAttachmentSize, const QString &_dmType,
	    enum MessageDirection msgDirect);

	/*!
	 * @brief Get message state.
	 *
	 * @param[in] dmId  Message identifier.
	 * @return Message state number or -1 on error.
	 */
	int messageState(qint64 dmId) const;

	/*!
	 * @brief Update message envelope delivery information.
	 *
	 * @param[in] dmId              Message identifier.
	 * @param[in] dmDeliveryTime    Delivery time in database format.
	 * @param[in] dmAcceptanceTime  Acceptance time in database format.
	 * @return True on success.
	 */
	bool msgsUpdateMessageState(qint64 dmId,
	    const QString &dmDeliveryTime, const QString &dmAcceptanceTime,
	    int dmMessageStatus);

	/*!
	 * @brief Insert/update message files into files table.
	 *
	 * @param[in] dmId                    Message identifier.
	 * @param[in] dmFileDescr             File name.
	 * @param[in] dmUpFileGuid
	 * @param[in] dmMimeType
	 * @param[in] dmFormat
	 * @param[in] dmFileMetaType
	 * @param[in] dmEncodedContentBase64  Base64-encoded file content.
	 * @return True on success.
	 */
	bool msgsInsertUpdateMessageFile(qint64 dmId,
	    const QString &dmFileDescr, const QString &dmUpFileGuid,
	    const QString &dmFileGuid, const QString &dmMimeType,
	    const QString &dmFormat, const QString &dmFileMetaType,
	    const QByteArray &dmEncodedContentBase64);

	/*!
	 * @brief Delete all files related to message with given id.
	 *
	 * @param[in] dmId Message identifier.
	 * @return True on success.
	 */
	bool flsDeleteMessageFiles(qint64 dmId);

	/*!
	 * @brief Insert/update message hash into hashes table.
	 *
	 * @param[in] dmId         Message identifier.
	 * @param[in] valueBase64  Base64-encoded hash value.
	 * @param[in] algorithm    Algorithm identifier.
	 * @return True on success.
	 */
	bool msgsInsertUpdateMessageHash(qint64 dmId,
	    const QByteArray &valueBase64, const QString &algorithm);

	/*!
	 * @brief Insert/update message events into events table.
	 *
	 * @param[in] dmId          Message identifier.
	 * @param[in] dmEventTime   Event time in database format.
	 * @param[in] dmEventType   Event type identifier.
	 * @param[in] dmEventDescr  Event description.
	 * @return True on success.
	 */
	bool msgsInsertUpdateMessageEvent(qint64 dmId,
	    const QString &dmEventTime, const QString &dmEventType,
	    const QString &dmEventDescr);

	/*!
	 * @brief Insert/update raw (DER) message data into raw_message_data
	 *     table.
	 *
	 * @param[in] dmId         Message identifier.
	 * @param[in] raw          Raw (non-base64 encoded) message data.
	 * @param[in] messageType  Message type.
	 * @return True on success.
	 */
	bool msgsInsertUpdateMessageRaw(qint64 dmId, const QByteArray &raw,
	    int messageType);

	/*!
	 * @brief Return all IDs of messages without attachment.
	 *
	 * @return Message identifier list.
	 */
	QList<qint64> getAllMessageIDsWithoutAttach(void) const;

	/*!
	 * @brief Return all message IDs from database.
	 *
	 * @param[in] messageType Specifies sent or received messages.
	 * @return Message identifier list.
	 */
	QList<qint64> getAllMessageIDs(enum MessageType messageType) const;

	/*!
	 * @brief Check whether whole message is stored in database.
	 *
	 * @param[in] dmId  Message identifier.
	 * @return True if whole message exists.
	 */
	bool msgsStoredWhole(qint64 dmId) const;

	/*!
	 * @brief Get base64 encoded raw message data.
	 *
	 * @param[in] dmId  Message identifier.
	 * @return Empty byte array on error.
	 */
	QByteArray msgsMessageBase64(qint64 dmId) const;

	/*!
	 * @brief Get message data in DER (raw) format.
	 *
	 * @param[in] dmId  Message identifier.
	 * @return Empty byte array on error.
	 */
	QByteArray msgsMessageRaw(qint64 dmId) const;

	/*!
	 * @brief Get base64-encoded delivery info from
	 *     raw_delivery_info_data table.
	 *
	 * @param[in] dmId  Message identifier.
	 * @return Empty byte array on error.
	 */
	QByteArray msgsGetDeliveryInfoBase64(qint64 dmId) const;

	/*!
	 * @brief Insert/update raw (DER) delivery info into
	 *     raw_delivery_info_data.
	 *
	 * @param[in] dmId  Message identifier.
	 * @param[in] raw   Raw (in DER format) delivery information.
	 * @return True on success.
	 */
	bool msgsInsertUpdateDeliveryInfoRaw(qint64 dmId,
	    const QByteArray &raw);

	/*!
	 * @brief Update information about author (sender).
	 *
	 * @param[in] dmId        Message identifier.
	 * @param[in] senderType  Type of sender.
	 * @param[in] senderName  Name of sender.
	 * @return True on success.
	 */
	bool updateMessageAuthorInfo(qint64 dmId, const QString &senderType,
	    const QString &senderName);

	/*!
	 * @brief Return hash of message from db.
	 *
	 * @param[in] dmId  Message identifier.
	 * @return Message hash structure.
	 */
	MessageHash msgsGetHashFromDb(qint64 dmId) const;

	/*!
	 * @brief Delete all message records from db.
	 *
	 * @param[in] dmId  Message identifier.
	 * @return True on success.
	 */
	bool msgsDeleteMessageData(qint64 dmId) const;

	/*!
	 * @brief Return some message items in order to export correspondence
	 *     to HTML.
	 *
	 * @param[in] dmId  Message identifier.
	 * @return String list containing sender, recipient, annotation, ...
	 *    Empty list is returned on error.
	 */
	QStringList getMsgForHtmlExport(qint64 dmId) const;

	/*!
	 * @brief Return some message items for export correspondence to csv.
	 *
	 * @param[in] dmId  Message identifier.
	 * @return String containing message status, message type, ...
	 *    Empty list is returned on error.
	 */
	QStringList getMsgForCsvExport(qint64 dmId) const;

	/*!
	 * @brief Set the verification result.
	 *
	 * @param[in] dmId Message identifier.
	 * @param[in] verified True is message was successfully verified,
	 *                     False if verification failed.
	 * @return True if update was successful.
	 */
	bool msgsSetVerified(qint64 dmId, bool verified);

	/*!
	 * @brief Set process state of received message.
	 *
	 * @param[in] dmId    Message identifier.
	 * @param[in] state   Message state to be set.
	 * @param[in] insert  Whether to insert or update an information.
	 * @return True if update/insert was successful.
	 */
	bool msgSetProcessState(qint64 dmId, enum MessageProcessState state,
	    bool insert);

	/*!
	 * @brief Get process state of received message.
	 *
	 * @param[in] dmId  Message identifier.
	 * @return Message processing state, -1 on error.
	 */
	int msgGetProcessState(qint64 dmId) const;

	/*!
	 * @brief Returns time stamp in raw (DER) format.
	 *
	 * @param[in] dmId  Message identifier.
	 * @return Qualified time stamp in DER format.
	 *     Empty byte array on error.
	 */
	QByteArray msgsTimestampRaw(qint64 dmId) const;

	/*!
	 * @brief Return some additional filename entries as
	 *        (dmDeliveryTime, dmAcceptanceTime, dmAnnotation, dmSender)
	 *
	 * @param[in] dmId  Message identifier.
	 * @return FilenameEntry struct.
	 */
	FilenameEntry msgsGetAdditionalFilenameEntry(qint64 dmId) const;

	/*!
	 * @brief Copy message data to account database from source database.
	 *
	 * @param[in] dmId  Message identifier.
	 * @param[in] sourceDbPath  Source db path.
	 * @return True if copy of message data was success.
	 */
	bool copyCompleteMsgDataToAccountDb(const QString &sourceDbPath,
	    qint64 msgId);

	/*!
	 * @brief Copy all messages correspond with
	 *        year and their records from tables into new db.
	 *
	 * @return Return success or fail.
	 */
	bool copyRelevantMsgsToNewDb(const QString &newDbFileName,
	   const QString &year);

protected: /* These function are used from within a database container. */
	/*!
	 * @brief Appends to received entry list data received from SQL query.
	 *
	 * @param[in,out] entryList List to add entries to.
	 * @param[in] query Query to read data from.
	 */
	static
	void appendRcvdEntryList(QList<RcvdEntry> &entryList, QSqlQuery &query);

	/*!
	 * @brief Return entries for all received messages.
	 *
	 * @return List of entries, empty list on failure.
	 */
	QList<RcvdEntry> msgsRcvdEntries(void) const;

	/*!
	 * @brief Return entries for received messages within past 90 days.
	 *
	 * @return List of entries, empty list on failure.
	 */
	QList<RcvdEntry> msgsRcvdEntriesWithin90Days(void) const;

	/*!
	 * @brief Return entries for received messages within given year.
	 *
	 * @param[in] year         Year number.
	 * @return List of entries, empty list on failure.
	 */
	QList<RcvdEntry> msgsRcvdEntriesInYear(const QString &year) const;

	/*!
	 * @brief Return list of years (strings) in database.
	 *
	 * @param[in] type    Whether to obtain sent or received messages.
	 * @param[in] sorting Sorting.
	 * @return List of years.
	 */
	QStringList msgsYears(enum MessageType type,
	    enum Sorting sorting) const;

	/*!
	 * @brief Return list of years and number of messages in database.
	 *
	 * @param[in] type    Whether to obtain sent or received messages.
	 * @param[in] sorting Sorting.
	 * @return List of years and counts.
	 */
	QList< QPair<QString, int> > msgsYearlyCounts(enum MessageType type,
	    enum Sorting sorting) const;

	/*!
	 * @brief Return number of unread messages received within past 90
	 *     days.
	 *
	 * @param[in] type Whether to obtain sent or received messages.
	 * @return Number of unread messages, -1 on error.
	 */
	int msgsUnreadWithin90Days(enum MessageType type) const;

	/*!
	 * @brief Return number of unread received messages in year.
	 *
	 * @param[in] type Whether to obtain sent or received messages.
	 * @param[in] year Year number.
	 * @return Number of unread messages, -1 on error.
	 */
	int msgsUnreadInYear(enum MessageType type,
	    const QString &year) const;

	/*!
	 * @brief Appends to sent entry list data received from SQL query.
	 *
	 * @param[in,out] entryList List to add entries to.
	 * @param[in] query Query to read data from.
	 */
	static
	void appendSntEntryList(QList<SntEntry> &entryList, QSqlQuery &query);

	/*!
	 * @brief Return entries for all sent messages.
	 *
	 * @return List of entries, empty list on failure.
	 */
	QList<SntEntry> msgsSntEntries(void) const;

	/*!
	 * @brief Return entries for all sent messages within past 90 days.
	 *
	 * @return List of entries, empty list on failure.
	 */
	QList<SntEntry> msgsSntEntriesWithin90Days(void) const;

	/*!
	 * @brief Return entries for sent messages within given year.
	 *
	 * @param[in] year         Year number.
	 * @return List of entries, empty list on failure.
	 */
	QList<SntEntry> msgsSntEntriesInYear(const QString &year) const;

	/*!
	 * @brief Set message read locally for all received messages.
	 *
	 * @param[in] read  New read status.
	 * @return True on success.
	 */
	bool smsgdtSetAllReceivedLocallyRead(bool read = true);

	/*!
	 * @brief Set message read locally for received messages in given year.
	 *
	 * @param[in] year  Year number.
	 * @param[in] read  New read status.
	 * @return True on success.
	 */
	bool smsgdtSetReceivedYearLocallyRead(const QString &year,
	    bool read = true);

	/*!
	 * @brief Set message read locally for recently received messages.
	 *
	 * @param[in] read  New read status.
	 * @return True on success.
	 */
	bool smsgdtSetWithin90DaysReceivedLocallyRead(bool read = true);

	/*!
	 * @brief Set process state of received messages.
	 *
	 * @param[in] state  Message state to be set.
	 * @return True if operation successful.
	 */
	bool msgSetAllReceivedProcessState(enum MessageProcessState state);

	/*!
	 * @brief Set process state of received messages in given year.
	 *
	 * @param[in] year   Year.
	 * @param[in] state  Message state to be set.
	 * @return True if operation successful.
	 */
	bool smsgdtSetReceivedYearProcessState(const QString &year,
	    enum MessageProcessState state);

	/*!
	 * @brief Set process state of recently received messages.
	 *
	 * @param[in] state  Message state to be set.
	 * @return True if operation successful.
	 */
	bool smsgdtSetWithin90DaysReceivedProcessState(
	    enum MessageProcessState state);

	/*!
	 * @brief Returns message identifier of message with given id number.
	 *
	 * @paran[in] dmId Message identification number.
	 * @return Message identifier containing the seeked id number.
	 *     If no such message is found then a message identifier
	 *     containing -1 dmId returned.
	 */
	MsgId msgsMsgId(qint64 dmId) const;

	/*!
	 * @brief Return contacts from message db.
	 *
	 * @return List of vectors containing recipientId, recipientName,
	 *     recipentAddress.
	 */
	QList<ContactEntry> uniqueContacts(void) const;

	/*!
	 * @brief Return all message ID from database.
	 *
	 * @return message id list.
	 */
	QList<MsgId> getAllMessageIDsFromDB(void) const;

	/*!
	 * @brief Get list of all message IDs corresponding with year.
	 *
	 * @return Return message ID list.
	 */
	QList<qint64> getAllMsgsIDEqualWithYear(const QString &year) const;

	/*!
	 * @brief Return list of message ids corresponding to given date
	 *     interval.
	 *
	 * @param[in] fromDate  Start date.
	 * @param[in] toDate    Stop date.
	 * @param[in] sent      True for sent messages, false for received.
	 * @return List of message ids. Empty list on error.
	 */
	QList<MsgId> msgsDateInterval(const QDate &fromDate,
	    const QDate &toDate, enum MessageDirection msgDirect) const;

	/*!
	 * @brief Advance message envelope search.
	 *
	 * @return message item list pass to search query.
	 */
	QList<SoughtMsg> msgsAdvancedSearchMessageEnvelope(
	    qint64 dmId,
	    const QString &dmAnnotation,
	    const QString &dbIDSender, const QString &dmSender,
	    const QString &dmAddress,
	    const QString &dbIDRecipient, const QString &dmRecipient,
	    const QString &dmSenderRefNumber,
	    const QString &dmSenderIdent,
	    const QString &dmRecipientRefNumber,
	    const QString &dmRecipientIdent,
	    const QString &dmToHands,
	    const QString &dmDeliveryTime, const QString &dmAcceptanceTime,
	    enum MessageDirection msgDirect) const;

	/*!
	 * @brief Get message envelope data from id.
	 *
	 * @return message data for message id.
	 */
	SoughtMsg msgsGetMsgDataFromId(const qint64 msgId) const;

	/*!
	 * @brief Test if imported message is relevent to account db.
	 *
	 * @param[in] dmId  Message identifier.
	 * @param[in] databoxId  Databox ID where message should be imported.
	 * @return Message is relevant for import to db or not.
	 */
	bool isRelevantMsgForImport(qint64 msgId, const QString databoxId) const;

	/*!
	 * @brief Open database file.
	 *
	 * @param[in] fileName       File name.
	 * @param[in] createMissing  Whether to create missing tables.
	 * @return True on success.
	 */
	bool openDb(const QString &fileName, bool createMissing = true);

	/*!
	 * @brief Copy db.
	 *
	 * @param[in] newFileName  New location name.
	 * @return True on success.
	 *
	 * @note The copy is continued to be used. Original is closed.
	 */
	bool copyDb(const QString &newFileName);

	/*!
	 * @brief Move db.
	 *
	 * @param[in] newFileName  New location name.
	 * @return True on success.
	 */
	bool moveDb(const QString &newFileName);

	/*!
	 * @brief Open a new empty database file.
	 *
	 * @note The old database file is left untouched.
	 */
	bool reopenDb(const QString &newFileName);

	/*!
	 * @brief Query received messages within past 90 days.
	 *
	 * @param[in,out] query Query already assigned to a database.
	 * @return True on success.
	 */
	static
	bool msgsRcvdWithin90DaysQuery(QSqlQuery &query);

	/*!
	 * @brief Query received messages within past 90 days.
	 *
	 * @param[in,out] query Query already assigned to a database.
	 * @return True on success.
	 */
	static
	bool msgsSntWithin90DaysQuery(QSqlQuery &query);

public:
	/*
	 * TODO -- Use static methods returning reference to static constant
	 * vector.
	 */
	static
	const QVector<QString> rcvdItemIds;
	static
	const QVector<QString> sntItemIds;
	static
	const QVector<QString> fileItemIds;

private:
	static
	const QVector<QString> msgPrintedAttribs;
	static
	const QVector<QString> msgDeliveryBoolAttribs;
	static
	const QVector<QString> msgStatus;

	/*!
	 * @brief Adds _dmType column.
	 *
	 * @return True on success.
	 *
	 * @note This code may be needed to update database between different
	 * versions.
	 */
	bool addDmtypeColumn(void);

	/*!
	 * @brief Returns list of tables.
	 *
	 * @return List of pointers to tables.
	 */
	static
	QList<class SQLiteTbl *> listOfTables(void);

	/*!
	 * @brief This method ensures that the process_state table
	 *     contains a PRIMARY KEY. This table might be created without any
	 *     primary key reference due to a bug in a previous version.
	 *
	 * @return True on success.
	 *
	 * TODO -- This method may be removed in some future version
	 *     of the programme.
	 */
	bool ensurePrimaryKeyInProcessStateTable(void);

	/*!
	 * @brief Returns verification date (in local time).
	 *
	 * @param[in] dmId  Message identifier.
	 * @return Message verification date. Invalid value is returned on
	 *     error.
	 */
	QDateTime msgsVerificationDate(qint64 dmId) const;

	/*!
	 * @brief Read data from supplementary message data table.
	 *
	 * @brief msgId  Message identifier.
	 * @return Stored json document data. Returns empty document on error.
	 */
	QJsonDocument smsgdCustomData(qint64 msgId) const;

	/*!
	 * @brief Check whether message signature was valid at given date
	 *     (local time).
	 *
	 * @param[in] dmId                  Message id.
	 * @param[in] dateTime              Local time identifier.
	 * @param[in] ignoreMissingCrlCheck Ignore CRL check if set to true.
	 * @return True if date check succeeds.
	 */
	bool msgCertValidAtDate(qint64 dmId, const QDateTime &dateTime,
	    bool ignoreMissingCrlCheck = false) const;

	/*!
	 * @brief Add/update message certificate in database.
	 *
	 * @brief[in] dmId       Message identifier.
	 * @brief[in] crtBase64  Base64-encoded certificate.
	 * @return True on success.
	 */
	bool msgsInsertUpdateMessageCertBase64(qint64 dmId,
	    const QByteArray &crtBase64);

	friend class MessageDbSet;
	friend class MessageDbSingle;
};

#endif /* _MESSAGE_DB_H_ */
