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

#ifndef QDJANGO_H
#define QDJANGO_H

class QSqlDatabase;
class QSqlQuery;
class QString;

class QDjangoModel;

void sqlDebug(const QSqlQuery &query);
bool sqlExec(QSqlQuery &query);

/** QDjango provides access to registered QDjangoModel classes.
 */
class QDjango
{
public:
    static void createTables();

    static const QDjangoModel *model(const QString &name);
    static bool registerModel(QDjangoModel *model);

    static QString quote(const QString &name);
    static QString unquote(const QString &quoted);
};

/** Register a QDjangoModel with QDjango.
 */
template <class T>
void qDjangoRegisterModel()
{
    if (!QDjango::model(T::staticMetaObject.className()))
    {
        T *model = new T;
        if (!QDjango::registerModel(model))
            delete model;
    }
}

#endif