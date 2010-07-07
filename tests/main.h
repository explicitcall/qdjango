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

#include "QDjango.h"

#include <QObject>

class Object : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString foo READ foo WRITE setFoo)
    Q_PROPERTY(int bar READ bar WRITE setBar)

public:
    QString foo() const;
    void setFoo(const QString &foo);

    int bar() const;
    void setBar(int bar);

private:
    QString m_foo;
    int m_bar;
};

class TestModel : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void save();
    void cleanupTestCase();

private:
    QDjangoMetaModel metaModel;
};

/** Test QDjangoWhere class.
 */
class TestWhere : public QObject
{
    Q_OBJECT

private slots:
    void emptyWhere();
    void equalsWhere();
    void notEqualsWhere();
    void greaterThan();
    void greaterOrEquals();
    void lessThan();
    void lessOrEquals();
    void isIn();
    void startsWith();
    void endsWith();
    void contains();
    void andWhere();
    void orWhere();
    void complexWhere();
};

