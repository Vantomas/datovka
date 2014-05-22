

#ifndef _DB_TABLES_H_
#define _DB_TABLES_H_


#include <QMap>
#include <QPair>
#include <QSqlDatabase>
#include <QString>
#include <QVector>


/*!
 * @brief Used data types in databases.
 */
enum EntryType{
	DB_INTEGER = 1,
	DB_TEXT,
	DB_BOOLEAN,
	DB_DATETIME
};


/*!
 * @brief Converts db types strings.
 */
const QString & entryTypeStr(EntryType entryType);


/*!
 * @brief Table attribute property.
 */
class AttrProp {
public:
	EntryType type; /*!< Attribute type. */
	QString desc; /*!< Attribute description. */
};


/*!
 * @brief Table prototype.
 */
class Tbl {
public:
	/*
	 * Constructor.
	 */
	Tbl(const QString &name,
	    const QVector< QPair<QString, EntryType> > &attrs,
	    const QMap<QString, AttrProp> &props,
	    const QMap<QString, QString> &colCons = Tbl::emptyColConstraints,
	    const QString &tblCons = emptyTblConstraint);

	/*! Table name. */
	const QString &tabName;

	/*! Known attributes. */
	const QVector< QPair<QString, EntryType> > &knownAttrs;

	/*! Attribute properties. */
	const QMap<QString, AttrProp> &attrProps;

	/*!
	 * @brief Return true if table in database exists.
	 */
	bool existsInDb(const QSqlDatabase &db) const;

	/*!
	 * @brief Create empty table in supplied database.
	 */
	bool createEmpty(QSqlDatabase &db) const;

private:
	/*! Column constraints. */
	const QMap<QString, QString> &colConstraints;

	/*! Table constraint. */
	const QString &tblConstraint;

	/*! Empty column constraints. */
	static
	const QMap<QString, QString> emptyColConstraints;

	/*! Empty table constraint. */
	static
	const QString emptyTblConstraint;
};


/*
 * Account database.
 */
extern const Tbl accntinfTbl; /*!< Table 'account_info'. */


/*
 * Message database.
 */
extern const Tbl msgsTbl; /*!< Table 'messages'. */
extern const Tbl flsTbl; /*!< Table 'files'. */
extern const Tbl hshsTbl; /*!< Table 'hashes'. */
extern const Tbl evntsTbl; /*!< Table 'events'. */
extern const Tbl rwmsgdtTbl; /*!< Table 'raw_message_data'. */
extern const Tbl rwdlvrinfdtTbl; /*!< Table 'raw_delivery_info_data'. */
extern const Tbl smsgdtTbl; /*!< Table 'supplementary_message_data'. */
extern const Tbl crtdtTbl; /*!< Table 'certificate_data'. */
extern const Tbl msgcrtdtTbl; /*!< Table 'message_certificate_data'. */


#endif /* _DB_TABLES_H_ */
