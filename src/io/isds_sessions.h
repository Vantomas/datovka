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

#ifndef _ISDS_SESSIONS_H_
#define _ISDS_SESSIONS_H_

#if defined(__APPLE__) || defined(__clang__)
#  define __USE_C99_MATH
#  define _Bool bool
#else /* !__APPLE__ */
#  include <cstdbool>
#endif /* __APPLE__ */

#include <isds.h>
#include <QMap>
#include <QString>

#include "src/settings/accounts.h"

/* TODO -- Check whether session is active. */

/* Global ISDS context container instance. */
class IsdsSessions;
extern IsdsSessions globIsdsSessions;

/*!
 * @brief Holds the ISDS context structures.
 */
class IsdsSessions {

public:
	IsdsSessions(void);
	~IsdsSessions(void);

	/*!
	 * @brief Returns true is active session exists.
	 */
	bool holdsSession(const QString &userName) const;

	/*!
	 * @brief Returns associated session.
	 */
	struct isds_ctx *session(const QString &userName) const;

	/*!
	 * @brief Ping of ISDS. Test if connection is active.
	 */
	bool isConnectedToIsds(const QString &userName);

	/*!
	 * @brief Creates new session.
	 *
	 * @return Pointer to new session or NULL on failure.
	 */
	struct isds_ctx * createCleanSession(const QString &userName,
	    unsigned int connectionTimeoutMs);

	/*!
	 * @brief Set time-out in milliseconds to session associated to
	 *     user name.
	 *
	 * @return True on success.
	 */
	bool setSessionTimeout(const QString &userName,
	    unsigned int timeoutMs);

private:
	QMap<QString, struct isds_ctx *> m_sessions;
};

/*!
 * @brief Log in using user name and password.
 */
isds_error isdsLoginUserName(struct isds_ctx *isdsSession,
    const QString &userName, const QString &pwd, bool testingSession);

/*!
 * @brief Log in using system certificate.
 */
isds_error isdsLoginSystemCert(struct isds_ctx *isdsSession,
    const QString &certPath, const QString &passphrase, bool testingSession);

/*!
 * @brief Log in using user certificate without password.
 * NOTE: It need ID of Databox instead username
 */
isds_error isdsLoginUserCert(struct isds_ctx *isdsSession,
    const QString &idBox, const QString &certPath, const QString &passphrase,
    bool testingSession);

/*!
 * @brief Log in using user certificate with password.
 */
isds_error isdsLoginUserCertPwd(struct isds_ctx *isdsSession,
    const QString &userName, const QString &pwd, const QString &certPath,
    const QString &passphrase, bool testingSession);

/*!
 * @brief Log in using username, pwd and OTP.
 */
isds_error isdsLoginUserOtp(struct isds_ctx *isdsSession,
    const QString &userName, const QString &pwd, bool testingSession,
    enum AcntSettings::LogInMethod otpMethod, const QString &otpCode,
    isds_otp_resolution &res);

/*!
 * @brief Add items into isds_PersonName structure.
 *
 * @return NULL on failure.
 */
struct isds_PersonName * isds_PersonName_create(const QString &pnFirstName,
    const QString &pnMiddleName, const QString &pnLastName,
    const QString &pnLastNameAtBirth);

/*!
 * @brief Add items into isds_Address structure.
 *
 * @return NULL on failure.
 */
struct isds_Address * isds_Address_create(const QString &adCity,
    const QString &adStreet, const QString &adNumberInStreet,
    const QString &adNumberInMunicipality, const QString &adZipCode,
    const QString &adState);

/*!
 * @brief Add items into isds_BirthInfo structure.
 *
 * @return NULL on failure.
 *
 * @note All structure pointers are consumed. The structures are not copied.
 *     Don't use the structures after a successful return from this function.
 */
struct isds_BirthInfo * isds_BirthInfo_createConsume(struct tm *biDate,
    const QString &biCity, const QString &biCountry, const QString &biState);

/*!
 * @brief Create new isds_DbOwnerInfo structure according to the supplied
 *     values.
 *
 * @return NULL on failure.
 *
 * @note All structure pointers are consumed. The structures are not copied.
 *     Don't use the structures after a successful return from this function.
 */
struct isds_DbOwnerInfo * isds_DbOwnerInfo_createConsume(const QString &dbID,
    isds_DbType dbType, const QString &ic,
    struct isds_PersonName *personName, const QString &firmName,
    struct isds_BirthInfo *birthInfo, struct isds_Address *address,
    const QString &nationality, const QString &email, const QString telNumber,
    const QString &identifier, const QString &registryCode, long int dbState,
    bool dbEffectiveOVM, bool dbOpenAddressing);

/*!
 * @brief Create DbUserInfo structure.
 *
 * @return NULL on failure.
 *
 * @note All structure pointers are consumed. The structures are not copied.
 *     Don't use the structures after a successful return from this function.
 */
struct isds_DbUserInfo * isds_DbUserInfo_createConsume(const QString &userID,
    isds_UserType userType, long int userPrivils,
    struct isds_PersonName *personName, struct isds_Address *address,
    const QString &ic, const QString &firmName, const QString &caStreet,
    const QString &caCity, const QString &caZipCode, const QString &caState);

/*!
 * @brief Creates a isds message from supplied raw CMS data.
 *
 * @param[in,out] isdsSession Pointer to session context.
 * @param[in]     rawMsgData  Raw message data.
 * @param[in]     zfoType     Message or delivery info (enum Imports::Type).
 * @return Pointer to newly allocated message, NULL on error. Use
 *     isds_message_free() to delete.
 */
struct isds_message *loadZfoData(struct isds_ctx *isdsSession,
    const QByteArray &rawMsgData, int zfoType);

/*!
 * @brief Create a isds message from zfo file.
 *
 * @param[in,out] isdsSession Pointer to session context.
 * @param[in]     fName       File name.
 * @param[in]     zfoType     Message or delivery info
 *                            (enum ImportZFODialog::ZFOtype).
 * @return Pointer to newly allocated message, NULL on error. Use
 *     isds_message_free() to delete.
 */
struct isds_message *loadZfoFile(struct isds_ctx *isdsSession,
    const QString &fName, int zfoType);

/*!
 * @brief Wraps isds_strerror().
 */
inline
QString isdsStrError(const isds_error error)
{
#ifdef WIN32
	/* The function returns strings in local encoding. */
	return QString::fromLocal8Bit(isds_strerror(error));
	/*
	 * TODO -- Is there a mechanism how to force the local encoding
	 * into libisds to be UTF-8?
	 */
#else /* !WIN32 */
	return QString::fromUtf8(isds_strerror(error));
#endif /* WIN32 */
}

/*!
 * @brief Wraps the isds_long_message().
 */
inline
QString isdsLongMessage(const struct isds_ctx *context)
{
#ifdef WIN32
	/* The function returns strings in local encoding. */
	return QString::fromLocal8Bit(isds_long_message(context));
	/*
	 * TODO -- Is there a mechanism how to force the local encoding
	 * into libisds to be UTF-8?
	 */
#else /* !WIN32 */
	return QString::fromUtf8(isds_long_message(context));
#endif /* WIN32 */
}

#endif /* _ISDS_SESSIONS_H_ */
