/*
 * QDjango
 * Copyright (C) 2010-2011 Bolloré telecom
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

#ifndef QDJANGO_MODEL_H
#define QDJANGO_MODEL_H

#include <QObject>
#include <QVariant>

/** \brief The QDjangoModel class is the base class for all models.
 *
 *  To declare your own model, create a class which inherits QDjangoModel
 *  and declare the database fields as properties using the Q_PROPERTY
 *  macro. You must then register the class with QDjango using
 *  QDjango::registerModel().
 *
 *  You can provide options for the model using the Q_CLASSINFO macro as
 *  follows:
 *
 *  \code
 *  Q_CLASSINFO("__meta__", "keyword1=value1 .. keywordN=valueN")
 *  \endcode
 *
 *  The following keywords are recognised for model options:
 *
 *  \li \c db_table if provided, this is the name of the database table for
 *  the model, otherwise the lowercased class name will be used
 *
 *  You can also provide additional information about a field using the
 *  Q_CLASSINFO macro, in the form:
 *
 *  \code
 *  Q_CLASSINFO("field_name", "keyword1=value1 .. keywordN=valueN")
 *  \endcode
 *
 *  The following keywords are recognised for field options:
 *
 *  \li \c auto_increment if set to 'true', and if this field is the primary
 *  key, it will be marked as auto-increment.
 *  \li \c db_index if set to 'true', an index will be created on this field.
 *  \li \c ignore_field if set to 'true', this field will be ignored
 *  \li \c max_length the maximum length of the field (used when creating
 *  the database table)
 *  \li \c primary_key if set to 'true', this field will be used as the primary
 *  key. If no primary key is explicitly defined, an auto-increment integer
 *  field will be added.
 *
 * \ingroup Database
 */
class QDjangoModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant pk READ pk WRITE setPk)
    Q_CLASSINFO("pk", "ignore_field=true")

public:
    QDjangoModel(QObject *parent = 0);

    QVariant pk() const;
    void setPk(const QVariant &pk);

public slots:
    bool remove();
    bool save();
    QString toString() const;

protected:
    QObject *foreignKey(const char *name) const;
    void setForeignKey(const char *name, QObject *value);
};

#endif
