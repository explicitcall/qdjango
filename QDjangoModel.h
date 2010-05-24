/*
 * QDjango
 * Copyright (C) 2010 Bolloré telecom
 * See AUTHORS file for a full list of contributors.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QDJANGO_MODEL_H
#define QDJANGO_MODEL_H

class QSqlDatabase;

#include <QMap>
#include <QObject>
#include <QVariant>

/** Base class for all models.
 *
 *  To declare your own model, create a class which inherits QDjangoModel
 *  and declare the database fields as properties using the Q_PROPERTY
 *  macro.
 *
 *  You must then register the class with QDjango using qDjangoRegisterModel.
 */
class QDjangoModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant pk READ pk WRITE setPk)

public:
    QDjangoModel(QObject *parent = 0);

    QVariant pk() const;
    void setPk(const QVariant &pk);

    static QSqlDatabase database();
    static void setDatabase(QSqlDatabase database);

public slots:
    bool createTable() const;
    bool dropTable() const;

    bool remove();
    bool save();

protected:
    void addForeignKey(const QString &field, QDjangoModel *model);
    QDjangoModel *foreignKey(const QString &field) const;

private:
    QString databaseColumn(const QString &name) const;
    void databaseLoad(const QMap<QString, QVariant> &props);
    QString databaseTable() const;
    QStringList databaseFields() const;

    int m_id;
    QString m_pkName;
    QMap<QString,QDjangoModel*> m_foreignModels;

    friend class QDjangoQueryBase;
};

#endif