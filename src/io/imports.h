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

#ifndef _IMPORTS_H_
#define _IMPORTS_H_

#include "src/gui/dlg_import_zfo.h"
#include "src/worker/task.h"

/*!
 * @brief Provides zfo and message import to local database.
 */
class Imports {

public:

	/*!
	 * @brief Import messages from external databases to local database.
	 *
	 * @param[in,out] parent - Parent widget to call object from.
	 * @param[in] dbSet - Account target database set.
	 * @param[in] dbFileList - List of external databases to import.
	 * @param[in] userName - Account username.
	 * @param[in] dbId - Databox ID for import.
	 * @param[out] errTxt - Error text.
	 */
	static
	void importDbMsgsIntoDatabase(QWidget *parent,
	    MessageDbSet &dbSet, const QStringList &dbFileList,
	    const QString &userName, const QString &dbId, QString &errTxt);

	/*!
	 * @brief Import ZFO file(s) into database by ZFO type.
	 *
	 * @param[in] fileList - List of file path to import.
	 * @param[in] databaseList- List of databases.
	 * @param[in] zfoType - ZFO type for import.
	 * @param[in] authenticate - Check ZFO validity in the ISDS.
	 * @param[out] zfoFilesToImport - List of valid ZFOs for import.
	 * @param[out] zfoFilesInvalid - List of invalid ZFOs.
	 * @param[out] numFilesToImport - Number of valid ZFOs for import.
	 * @param[out] errTxt - Error text.
	 */
	static
	void importZfoIntoDatabase(const QStringList &fileList,
	    const QList<Task::AccountDescr> &databaseList,
	    enum ImportZFODialog::ZFOtype zfoType, bool authenticate,
	    QSet<QString> &zfoFilesToImport,
	    QList< QPair<QString, QString> > &zfoFilesInvalid,
	    int &numFilesToImport, QString &errTxt);

private:
	/*!
	 * @brief Private constructor.
	 *
	 * @note Just prevent any instances of this class.
	 */
	Imports(void);

	/*!
	 * @brief Show error/result message box.
	 *
	 * @param[in,out] parent - Parent widget to call object from.
	 * @param[in] title - Message box title.
	 * @param[in] baseText - Message box base text.
	 * @param[in] informativeText - Message box informative text.
	 * @param[in] detailedText - Message box detail text.
	 */
	static
	void showMessageBox(QWidget *parent, const QString &title,
	    const QString &baseText, const QString &informativeText,
	    const QString &detailedText);
};

#endif /* _IMPORTS_H_ */
