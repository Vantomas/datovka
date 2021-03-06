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

#ifndef _ISDS_CONVERSION_H_
#define _ISDS_CONVERSION_H_

#include <QCoreApplication> // Q_DECLARE_TR_FUNCTIONS
#include <QString>

/*!
 * @brief Provides conversion functions for ISDS types.
 */
class IsdsConversion {
	Q_DECLARE_TR_FUNCTIONS(IsdsConversion)

private:
	/*!
	 * @brief Private constructor.
	 */
	IsdsConversion(void);

public:
	/*!
	 * @brief Return attachment type as string.
	 *
	 * @param[in] val Attachment type value as used by libisds.
	 * @return Attachment type description.
	 */
	static
	const QString &attachmentTypeToStr(int val);

	/*!
	 * @brief Convert data box state to localised text
	 *
	 * @param[in] val Data box status value as used by libisds.
	 * @return Data box status description string.
	 */
	static
	QString boxStateToText(int val);

	/*!
	 * @brief Convert data box type to string.
	 *
	 * @param[in] val Data box type value as used by libisds.
	 * @return Data box type description.
	 */
	static
	const QString &boxTypeToStr(int val);

	/*!
	 * @brief Convert data box type string to int as used by libisds.
	 *
	 * @param[in] val Data box type string.
	 * @return Data box type integer value.
	 */
	static
	int boxTypeStrToInt(const QString &val);

	/*!
	 * @brief Return localised sender data box type description string.
	 *
	 * @param[in] val Sender data box type value as used by libisds.
	 * @return Sender data box type description.
	 */
	static
	QString senderBoxTypeToText(int val);

	/*!
	 * @brief Translates message type to localised text.
	 *
	 * @param[in] val Message type value as used by libisds.
	 * @return Message type description.
	 */
	static
	QString dmTypeToText(const QString &val);

	/*!
	 * @brief Convert event type to string.
	 *
	 * @param[in] val Event type value as used by libisds.
	 * @return Event string (not the description).
	 */
	static
	const QString &eventTypeToStr(int val);

	/*!
	 * @brief Return hash algorithm string identifier.
	 *
	 * @param[in] val Hash algorithm value as used by libisds.
	 * @return Hash algorithm type string.
	 */
	static
	const QString &hashAlgToStr(int val);

	/*!
	 * @brief Converts hash algorithm type string to int as used by libisds.
	 *
	 * @param[in] val Hash algorithm string.
	 * @return Hash algorithm type integer value.
	 */
	static
	int hashAlgStrToInt(const QString &val);

	/*!
	 * @brief Converts libisds message state to database value as used by
	 *     Datovka.
	 *
	 * @param[in] val Message status value as used by libisds.
	 * @return Datovka status value as stored in database.
	 */
	static
	int msgStatusIsdsToDbRepr(int val);

	/*!
	 * @brief Returns localised message status description text.
	 *
	 * @param[in] val Message status value as used in database.
	 * @return Localised message status description.
	 */
	static
	QString msgStatusDbToText(int val);

	/*!
	 * @brief Convert sender type to string identifier.
	 *
	 * @param[in] val Sender type value as used by libisds.
	 * @return Sender type string identifier.
	 */
	static
	const QString &senderTypeToStr(int val);

	/*!
	 * @brief Translates sender type string to localised text.
	 *
	 * @param[in] val Sender type string.
	 * @return Sender type description.
	 */
	static
	QString senderTypeStrToText(const QString &val);

	/*!
	 * @brief Convert type of user to string.
	 *
	 * @param[in] val User type value as used by libisds.
	 * @return User type string.
	 */
	static
	const QString &userTypeToStr(int val);

	/*!
	 * @brief Return privileges as html string from number representation.
	 *
	 * @param[in] val Privilege value flags.
	 * @return Privilege description.
	 */
	static
	QString userPrivilsToText(int val);
};

#endif /* _ISDS_CONVERSION_H_ */
