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

#include <QSqlQuery>

#include "QDjango.h"
#include "QDjangoModel.h"
#include "QDjangoQuerySet.h"

typedef QMap<QString, QVariant> PropertyMap;

QDjangoQueryBase::QDjangoQueryBase(const QString &modelName)
    : m_lowMark(0),
    m_highMark(0),
    m_haveResults(false),
    m_modelName(modelName)
{
}

QStringList QDjangoQueryBase::fieldNames(const QDjangoModel *model, QString &from)
{
    QStringList fields;
    foreach (const QString &field, model->databaseFields())
        fields.append(model->databaseColumn(field));
    if (!m_selectRelated)
        return fields;

    // recurse for foreign keys
    foreach (const QString &fk, model->m_foreignModels.keys())
    {
        const QDjangoModel *foreign = model->m_foreignModels[fk];
        fields += fieldNames(foreign, from);
        from += QString(" INNER JOIN %1 ON %2 = %3")
            .arg(QDjango::quote(foreign->databaseTable()))
            .arg(foreign->databaseColumn(foreign->m_pkName))
            .arg(model->databaseColumn(model->m_foreignKeys[fk]));
    }
    return fields;
}

void QDjangoQueryBase::addFilter(const QDjangoWhere &where)
{
    // it is not possible to add filters once a limit has been set
    Q_ASSERT(!m_lowMark && !m_highMark);

    const QDjangoModel *model = QDjango::model(m_modelName);
    QDjangoWhere q(where);
    q.resolve(model);
    m_where = m_where && q;
}

void QDjangoQueryBase::sqlDelete()
{
    // delete entries
    const QDjangoModel *model = QDjango::model(m_modelName);
    QString from = QDjango::quote(model->databaseTable());
    QString sql = "DELETE FROM " + from;
    QString where = m_where.sql();
    if (!where.isEmpty())
        sql += " WHERE " + where;
    sql += sqlLimit();
    QSqlQuery query(QDjangoModel::database());
    query.prepare(sql);
    m_where.bindValues(query);
    sqlExec(query);

    // invalidate cache
    if (m_haveResults)
    {
        m_properties.clear();
        m_haveResults = false;
    }
}

void QDjangoQueryBase::sqlFetch()
{
    if (m_haveResults)
        return;

    // build query
    const QDjangoModel *model = QDjango::model(m_modelName);
    QString from = QDjango::quote(model->databaseTable());
    QStringList fields = fieldNames(model, from);
    QString sql = "SELECT " + fields.join(", ") + " FROM " + from;
    QString where = m_where.sql();
    if (!where.isEmpty())
        sql += " WHERE " + where;
    sql += sqlLimit();
    QSqlQuery query(QDjangoModel::database());
    query.prepare(sql);
    m_where.bindValues(query);

    // store results
    if (sqlExec(query))
    {
        while (query.next())
        {
            QMap<QString, QVariant> props;
            for (int i = 0; i < fields.size(); ++i)
                props.insert(fields[i], query.value(i));
            m_properties.append(props);
        }
    }
    m_haveResults = true;
}

QString QDjangoQueryBase::sqlLimit() const
{
    QString limit;
    if (m_highMark > 0)
        limit = QString(" LIMIT %1").arg(m_highMark - m_lowMark);
    if (m_lowMark > 0)
    {
        if (m_highMark <= 0)
        {
            const QString driverName = QDjangoModel::database().driverName();
            if (driverName == "QSQLITE" || driverName == "QSQLITE2")
                limit = " LIMIT -1";
            else if (driverName == "QMYSQL")
                // 2^64 - 1, as recommended by the MySQL documentation
                limit = " LIMIT 18446744073709551615";
        }
        limit += QString(" OFFSET %1").arg(m_lowMark);
    }
    return limit;
}

bool QDjangoQueryBase::sqlLoad(QDjangoModel *model, int index)
{
    sqlFetch();

    if (index < 0 | index >= m_properties.size())
    {
        qWarning("QDjangoQuerySet out of bounds");
        return false;
    }

    model->databaseLoad(m_properties.at(index));
    return true;
}

QList< QMap<QString, QVariant> > QDjangoQueryBase::sqlValues(const QStringList &fields)
{
    QList< QMap<QString, QVariant> > values;
    sqlFetch();

    // process local fields
    const QDjangoModel *model = QDjango::model(m_modelName);
    const QStringList fieldNames = fields.isEmpty() ? model->databaseFields() : fields;
    foreach (const PropertyMap &props, m_properties)
    {
        QMap<QString, QVariant> map;
        foreach (const QString &field, fieldNames)
        {
            const QString key = model->databaseColumn(field);
            map[field] = props[key];
        }
        values.append(map);
    }
    return values;
}

QList< QList<QVariant> > QDjangoQueryBase::sqlValuesList(const QStringList &fields)
{
    QList< QList<QVariant> > values;
    sqlFetch();

    const QDjangoModel *model = QDjango::model(m_modelName);
    const QStringList fieldNames = fields.isEmpty() ? model->databaseFields() : fields;
    foreach (const PropertyMap &props, m_properties)
    {
        QList<QVariant> list;
        foreach (const QString &field, fieldNames)
        {
            const QString key = model->databaseColumn(field);
            list << props.value(key);
        }
        values.append(list);
    }
    return values;
}

