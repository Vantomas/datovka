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

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include "src/io/db_tables.h"
#include "src/io/tag_db.h"
#include "src/log/log.h"


/* ========================================================================= */
/*
 * Constructor.
 */
TagItem::TagItem(void)
/* ========================================================================= */
    : id(-1),
    name(),
    colour()
{
}


/* ========================================================================= */
/*
 * Constructor.
 */
TagItem::TagItem(int i, const QString &n, const QString &c)
/* ========================================================================= */
    : id(i),
    name(n),
    colour(c)
{
}


/* ========================================================================= */
/*
 * Are valid tag data.
 */
bool TagItem::isValid(void) const
/* ========================================================================= */
{
	return (id >= 0) && !name.isEmpty() && (6 == colour.size());
}


/* ========================================================================= */
/*
 * Constructor.
 */
TagDb::TagDb(const QString &connectionName)
/* ========================================================================= */
    : SQLiteDb(connectionName)
{
}


/* ========================================================================= */
/*
 * Open tag database.
 */
bool TagDb::openDb(const QString &fileName)
/* ========================================================================= */
{
	return SQLiteDb::openDb(fileName, false, listOfTables());
}


/* ========================================================================= */
/*
 * Insert a new tag into tag table if not exists (tagName).
 */
bool TagDb::insertTag(const QString &tagName, const QString &tagColor)
/* ========================================================================= */
{
	QSqlQuery query(m_db);

	QString queryStr = "SELECT id FROM tag WHERE tag_name = :tag_name";

	if (!query.prepare(queryStr)) {
		logErrorNL("Cannot prepare SQL query: %s.",
		    query.lastError().text().toUtf8().constData());
		return false;
	}

	query.bindValue(":tag_name", tagName);

	if (query.exec() && query.isActive()) {
		query.first();
		if (query.isValid()) {
			return false;
		}
	}

	queryStr = "INSERT INTO tag (tag_name, tag_color) "
	    "VALUES (:tag_name, :tag_color)";

	if (!query.prepare(queryStr)) {
		logErrorNL("Cannot prepare SQL query: %s.",
		    query.lastError().text().toUtf8().constData());
		return false;
	}

	query.bindValue(":tag_name", tagName);
	query.bindValue(":tag_color", tagColor);

	if (!query.exec()) {
		logErrorNL("Cannot execute SQL query: %s.",
		    query.lastError().text().toUtf8().constData());
		return false;
	}

	return true;
}


/* ========================================================================= */
/*
 * Update tag in tag table.
 */
bool TagDb::updateTag(int id, const QString &tagName, const QString &tagColor)
/* ========================================================================= */
{
	QSqlQuery query(m_db);

	QString queryStr = "UPDATE tag SET tag_name = :tag_name, "
	    "tag_color = :tag_color WHERE id = :id";

	if (!query.prepare(queryStr)) {
		logErrorNL("Cannot prepare SQL query: %s.",
		    query.lastError().text().toUtf8().constData());
		return false;
	}

	query.bindValue(":tag_name", tagName);
	query.bindValue(":tag_color", tagColor);
	query.bindValue(":id", id);

	if (!query.exec()) {
		logErrorNL("Cannot execute SQL query: %s.",
		    query.lastError().text().toUtf8().constData());
		return false;
	}

	return true;
}


/* ========================================================================= */
/*
 * Delete tag from table.
 */
bool TagDb::deleteTag(int id)
/* ========================================================================= */
{
	QSqlQuery query(m_db);

	QString queryStr = "DELETE FROM message_tags WHERE tag_id = :id";
	if (!query.prepare(queryStr)) {
		logErrorNL("Cannot prepare SQL query: %s.",
		    query.lastError().text().toUtf8().constData());
		return false;
	}
	query.bindValue(":id", id);
	if (!query.exec()) {
		logErrorNL("Cannot execute SQL query: %s.",
		    query.lastError().text().toUtf8().constData());
		return false;
	}

	queryStr = "DELETE FROM tag WHERE id = :id";
	if (!query.prepare(queryStr)) {
		logErrorNL("Cannot prepare SQL query: %s.",
		    query.lastError().text().toUtf8().constData());
		return false;
	}
	query.bindValue(":id", id);
	if (!query.exec()) {
		logErrorNL("Cannot execute SQL query: %s.",
		    query.lastError().text().toUtf8().constData());
		return false;
	}

	return true;
}


/* ========================================================================= */
/*
 * Get tag data from table.
 */
TagItem TagDb::getTagData(int id)
/* ========================================================================= */
{
	QSqlQuery query(m_db);

	QString queryStr = "SELECT tag_name, tag_color FROM tag WHERE id = :id";
	if (!query.prepare(queryStr)) {
		logErrorNL("Cannot prepare SQL query: %s.",
		    query.lastError().text().toUtf8().constData());
		goto fail;
	}
	query.bindValue(":id", id);
	if (query.exec() && query.isActive() &&
	    query.first() && query.isValid()) {
		return TagItem(id, query.value(0).toString(),
		    query.value(1).toString());
	} else {
		logErrorNL(
		    "Cannot execute SQL query and/or read SQL data: %s.",
		    query.lastError().text().toUtf8().constData());
		goto fail;
	}

fail:
	return TagItem();
}


/* ========================================================================= */
/*
 * Get all tags from table.
 */
QList<TagItem> TagDb::getAllTags(void)
/* ========================================================================= */
{
	QSqlQuery query(m_db);
	QList<TagItem> tagList;

	QString queryStr = "SELECT * FROM tag ORDER BY tag_name ASC";
	if (!query.prepare(queryStr)) {
		logErrorNL("Cannot prepare SQL query: %s.",
		    query.lastError().text().toUtf8().constData());
		goto fail;
	}

	if (query.exec() && query.isActive()) {
		query.first();
		while (query.isValid()) {
			tagList.append(TagItem(query.value(0).toInt(),
			    query.value(1).toString(),
			    query.value(2).toString()));
			query.next();
		}
	} else {
		logErrorNL("Cannot execute SQL query: %s.",
		    query.lastError().text().toUtf8().constData());
		goto fail;
	}

	return tagList;
fail:
	return QList<TagItem>();
}


/* ========================================================================= */
/*
 * Delete all tags for message ID in the message_tags table.
 */
bool TagDb::removeAllTagsFromMsg(qint64 msgId)
/* ========================================================================= */
{
	QSqlQuery query(m_db);

	QString queryStr = "DELETE FROM message_tags WHERE message_id = :msgId";
	if (!query.prepare(queryStr)) {
		logErrorNL("Cannot prepare SQL query: %s.",
		    query.lastError().text().toUtf8().constData());
		return false;
	}
	query.bindValue(":msgId", msgId);
	if (!query.exec()) {
		logErrorNL("Cannot execute SQL query: %s.",
		    query.lastError().text().toUtf8().constData());
		return false;
	}

	return true;
}


/* ========================================================================= */
/*
 * Assign existing tag to message.
 */
bool TagDb::assignTagToMsg(int tagId, qint64 msgId)
/* ========================================================================= */
{
	QSqlQuery query(m_db);

	QString queryStr = "INSERT INTO message_tags (message_id, tag_id) "
	    "VALUES (:msgId, :tagId)";

	if (!query.prepare(queryStr)) {
		logErrorNL("Cannot prepare SQL query: %s.",
		    query.lastError().text().toUtf8().constData());
		return false;
	}

	query.bindValue(":msgId", msgId);
	query.bindValue(":tagId", tagId);

	if (!query.exec()) {
		logErrorNL("Cannot execute SQL query: %s.",
		    query.lastError().text().toUtf8().constData());
		return false;
	}
	return true;
}


/* ========================================================================= */
/*
 * Remove tag from message.
 */
bool TagDb::removeTagFromMsg(int tagId, qint64 msgId)
/* ========================================================================= */
{
	QSqlQuery query(m_db);

	QString queryStr = "DELETE FROM message_tags WHERE "
	    "message_id = :msgId AND tag_id = :tagId";
	if (!query.prepare(queryStr)) {
		logErrorNL("Cannot prepare SQL query: %s.",
		    query.lastError().text().toUtf8().constData());
		return false;
	}

	query.bindValue(":msgId", msgId);
	query.bindValue(":tagId", tagId);

	if (!query.exec()) {
		logErrorNL("Cannot execute SQL query: %s.",
		    query.lastError().text().toUtf8().constData());
		return false;
	}
	return true;
}


/* ========================================================================= */
/*
 * List of pointers to tables.
 */
QList<class SQLiteTbl *> TagDb::listOfTables(void)
/* ========================================================================= */
{
	static QList<class SQLiteTbl *> tables;
	if (tables.isEmpty()) {
		tables.append(&tagTbl);
		tables.append(&msgtagsTbl);
	}
	return tables;
}

/* Global poninter to tag database */
TagDb *globTagDbPtr = 0;
