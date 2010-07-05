/*
 * QDjango
 * Copyright (C) 2010 Bolloré telecom
 * See AUTHORS file for a full list of contributors.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QCoreApplication>
#include <QDebug>
#include <QMetaProperty>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QThread>

#include "QDjango.h"
#include "QDjango_p.h"
#include "QDjangoModel.h"

QMap<QString, QDjangoMetaModel> QDjango::metaModels = QMap<QString, QDjangoMetaModel>();

static QDjangoWatcher *globalWatcher = 0;

QDjangoWatcher::QDjangoWatcher(QObject *parent)
    : QObject(parent), connectionId(0)
{
}

void QDjangoWatcher::threadFinished()
{
    QThread *thread = qobject_cast<QThread*>(sender());
    copies.remove(thread);
}

static void closeDatabase()
{
    delete globalWatcher;
}

/*! \mainpage
 *
 * QDjango is a simple yet powerful Object Relation Mapper (ORM) built
 * on top of the Qt library. Where possible it tries to follow django's
 * ORM API, hence its name.
 *
 * \sa QDjango
 * \sa QDjangoModel
 * \sa QDjangoWhere
 * \sa QDjangoQuerySet
 */


static void sqlDebug(const QSqlQuery &query)
{
    qDebug() << "SQL query" << query.lastQuery();
    QMapIterator<QString, QVariant> i(query.boundValues());
    while (i.hasNext()) {
        i.next();
        qDebug() << "   " << i.key().toAscii().data() << "="
                 << i.value().toString().toAscii().data();
    }
}

bool sqlExec(QSqlQuery &query)
{
#ifdef QDJANGO_DEBUG_SQL
    sqlDebug(query);
    if (!query.exec())
    {
        qWarning() << "SQL error" << query.lastError();
        return false;
    }
    return true;
#else
    return query.exec();
#endif
}

/** Returns the database used by QDjango.
 *
 *  If you call this method from any thread but the application's main thread,
 *  a new connection to the database will be created. The connection will
 *  automatically be torn down once the thread finishes.
 *
 *  \sa setDatabase()
 */
QSqlDatabase QDjango::database()
{
    Q_ASSERT(globalWatcher != 0);
    QThread *thread = QThread::currentThread();

    // if we are in the main thread, return reference connection
    if (thread == globalWatcher->thread())
        return globalWatcher->reference;

    // if we have a connection for this thread, return it
    if (globalWatcher->copies.contains(thread))
        return globalWatcher->copies[thread];

    // create a new connection for this thread
    QObject::connect(thread, SIGNAL(finished()), globalWatcher, SLOT(threadFinished()));
    QSqlDatabase db = QSqlDatabase::cloneDatabase(globalWatcher->reference,
        QString("_qdjango_%1").arg(globalWatcher->connectionId++));
    Q_ASSERT(db.open());
    globalWatcher->copies.insert(thread, db);
    return db;
}

/** Sets the database used by QDjango.
 *
 *  You must call this method from your application's main thread.
 *
 *  \sa database()
 */
void QDjango::setDatabase(QSqlDatabase database)
{
    if (database.driverName() != "QSQLITE" &&
        database.driverName() != "QSQLITE2" &&
        database.driverName() != "QMYSQL")
    {
        qWarning() << "Unsupported database driver" << database.driverName();
    }
    if (!globalWatcher)
    {
        globalWatcher = new QDjangoWatcher();
        qAddPostRoutine(closeDatabase);
    }
    globalWatcher->reference = database;
}

/** Creates the database tables for all registered models.
 */
void QDjango::createTables()
{
    foreach (const QString &key, metaModels.keys())
        metaModels[key].createTable();
}

/** Drops the database tables for all registered models.
 */
void QDjango::dropTables()
{
    foreach (const QString &key, metaModels.keys())
        metaModels[key].dropTable();
}

/** Returns the QDjangoMetaModel with the given name.
 *
 * @param name
 */
QDjangoMetaModel QDjango::metaModel(const QString &name)
{
    return metaModels.value(name);
}

bool QDjango::registerModel(QDjangoModel *model)
{
    const QString name = model->metaObject()->className();
    if (metaModels.contains(name))
    {
        delete model;
        return false;
    }
    metaModels.insert(name, QDjangoMetaModel(model));
}

/** Returns the SQL used to declare a field as auto-increment.
 */
QString QDjango::autoIncrementSql()
{
    const QString driverName = QDjango::database().driverName();
    if (driverName == "QSQLITE" || driverName == "QSQLITE2")
        return QLatin1String(" AUTOINCREMENT");
    else if (driverName == "QMYSQL")
        return QLatin1String(" AUTO_INCREMENT");
    else
        return QString();
}

/** Returns the empty SQL limit clause.
 */
QString QDjango::noLimitSql()
{
    const QString driverName = QDjango::database().driverName();
    if (driverName == "QSQLITE" || driverName == "QSQLITE2")
        return QLatin1String(" LIMIT -1");
    else if (driverName == "QMYSQL")
        // 2^64 - 1, as recommended by the MySQL documentation
        return QLatin1String(" LIMIT 18446744073709551615");
    else
        return QString();
}

/** Quotes a database table or column name.
 */
QString QDjango::quote(const QString &name)
{
    return "`" + name + "`";
}

/** Unquotes a database table or column name.
 */
QString QDjango::unquote(const QString &quoted)
{
    if (quoted.startsWith("`") && quoted.endsWith("`"))
        return quoted.mid(1, quoted.size() - 2);
    return quoted;
}

QDjangoMetaField::QDjangoMetaField()
    : autoIncrement(false),
    index(false),
    maxLength(0),
    primaryKey(false)
{
}

QDjangoMetaModel::QDjangoMetaModel(const QDjangoModel *model)
    : m_model(model)
{
    const QMetaObject* meta = model->metaObject();
    m_table = QString(meta->className()).toLower();

    // local fields
    const int count = meta->propertyCount();
    for(int i = meta->propertyOffset(); i < count; ++i)
    {
        QDjangoMetaField field;
        field.name = meta->property(i).name();
        field.type = meta->property(i).type();

        // FIXME get rid of reference to model
        const QString fkName = m_model->m_foreignKeys.key(field.name);
        if (!fkName.isEmpty())
        {
            QDjangoModel *foreign = model->m_foreignModels[fkName];
            field.foreignModel = foreign->metaObject()->className();
            field.index = true;
        }

        // parse options
        const int infoIndex = meta->indexOfClassInfo(meta->property(i).name());
        if (infoIndex >= 0)
        {
            QMetaClassInfo classInfo = meta->classInfo(infoIndex);
            QStringList items = QString(classInfo.value()).split(' ');
            foreach (const QString &item, items)
            {
                QStringList assign = item.split('=');
                if (assign.length() == 2)
                {
                    const QString key = assign[0].toLower();
                    const QString value = assign[1];
                    if (key == "max_length")
                        field.maxLength = value.toInt();
                    else if (key == "primary_key")
                    {
                        field.index = true;
                        field.primaryKey = true;

                        m_primaryKey = field.name;
                    }
                }
            }
        }

        localFields << field;
    }

    // automatic primary key
    if (m_primaryKey.isEmpty())
    {
        QDjangoMetaField field;
        field.name = "id";
        field.type = QVariant::Int;
        field.autoIncrement = true;
        field.index = true;
        field.primaryKey = true;
        localFields.prepend(field);

        m_primaryKey = field.name;
    }
 
}

/** Creates the database table for this QDjangoModel.
 */
bool QDjangoMetaModel::createTable() const
{
    QSqlDatabase db = QDjango::database();
//    const QMetaObject* meta = metaObject();

    QStringList propSql;
    foreach (const QDjangoMetaField &field, localFields)
    {
        QString fieldSql = QDjango::quote(field.name);
        if (field.type == QVariant::Bool)
            fieldSql += " BOOLEAN";
        else if (field.type == QVariant::ByteArray)
        {
            fieldSql += " BLOB";
            if (field.maxLength > 0)
                fieldSql += QString("(%1)").arg(field.maxLength);
        }
        else if (field.type == QVariant::Date)
            fieldSql += " DATE";
        else if (field.type == QVariant::DateTime)
            fieldSql += " DATETIME";
        else if (field.type == QVariant::Double)
            fieldSql += " REAL";
        else if (field.type == QVariant::Int)
            fieldSql += " INTEGER";
        else if (field.type == QVariant::LongLong)
            fieldSql += " INTEGER";
        else if (field.type == QVariant::String)
        {
            if (field.maxLength > 0)
                fieldSql += QString(" VARCHAR(%1)").arg(field.maxLength);
            else
                fieldSql += " TEXT";
        }
        else {
            qWarning() << "Unhandled type" << field.type << "for property" << field.name;
            continue;
        }

        // primary key
        if (field.primaryKey)
            fieldSql += " PRIMARY KEY";

        // auto-increment is backend specific
        if (field.autoIncrement)
            fieldSql += QDjango::autoIncrementSql();

        // foreign key
        if (!field.foreignModel.isEmpty())
        {
            const QDjangoMetaModel foreignMeta = QDjango::metaModel(field.foreignModel);
            fieldSql += QString(" REFERENCES %1 (%2)").arg(
                QDjango::quote(foreignMeta.m_table), QDjango::quote(foreignMeta.m_primaryKey));
        }
        propSql << fieldSql;
    }

    // create table
    QSqlQuery createQuery(db);
    createQuery.prepare(QString("CREATE TABLE %1 (%2)").arg(QDjango::quote(m_table), propSql.join(", ")));
    if (!sqlExec(createQuery))
        return false;

    // create indices
    foreach (const QDjangoMetaField &field, localFields)
    {
        if (field.index)
        {
            const QString indexName = QString("%1_%2").arg(m_table, field.name);
            QSqlQuery indexQuery(db);
            indexQuery.prepare(QString("CREATE %1 %2 ON %3 (%4)").arg(
                field.primaryKey ? "UNIQUE INDEX" : "INDEX",
                QDjango::quote(indexName),
                QDjango::quote(m_table),
                QDjango::quote(field.name)));
            if (!sqlExec(indexQuery))
                return false;
        }
    }

    return true;
}

/** Returns the database column for the given field.
 */
QString QDjangoMetaModel::databaseColumn(const QString &name, bool *needsJoin) const
{
    // foreign key lookup
    if (name.count("__"))
    {
        QStringList bits = name.split("__");
        QString fk = bits.takeFirst();
        if (m_model->m_foreignModels.contains(fk))
        {
            QDjangoModel *foreign = m_model->m_foreignModels[fk];
            const QDjangoMetaModel foreignMeta = QDjango::metaModel(foreign->metaObject()->className());
            if (needsJoin)
                *needsJoin = true;
            return foreignMeta.databaseColumn(bits.join("__"));
        }
    }

    QString realName = (name == "pk") ? m_primaryKey : name;
    return QDjango::quote(m_table) + "." + QDjango::quote(realName);
}

/** Drops the database table for this QDjangoMetaModel.
 */
bool QDjangoMetaModel::dropTable() const
{
    QSqlQuery query(QDjango::database());
    query.prepare(QString("DROP TABLE %1").arg(QDjango::quote(m_table)));
    return sqlExec(query);
}

