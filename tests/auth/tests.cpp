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

#include <QtTest/QtTest>

#include "QDjangoQuerySet.h"
#include "QDjangoWhere.h"

#include "models.h"
#include "tests.h"

void TestModel::initTestCase()
{
    QCOMPARE(User().createTable(), true);
}

void TestModel::createUser()
{
    const QDjangoQuerySet<User> users;
    User *other;

    // create
    User user;
    user.setUsername("foouser");
    user.setPassword("foopass");
    user.setLastLogin(QDateTime(QDate(2010, 6, 1), QTime(10, 5, 14)));
    QCOMPARE(user.save(), true);
    QCOMPARE(users.all().size(), 1);

    // get by id
    other = users.get(QDjangoWhere("id", QDjangoWhere::Equals, 1));
    QVERIFY(other != 0);
    QCOMPARE(other->pk(), QVariant(1));
    QCOMPARE(other->username(), QLatin1String("foouser"));
    QCOMPARE(other->password(), QLatin1String("foopass"));
    delete other;

    // get by pk
    other = users.get(QDjangoWhere("pk", QDjangoWhere::Equals, 1));
    QVERIFY(other != 0);
    QCOMPARE(other->pk(), QVariant(1));
    QCOMPARE(other->username(), QLatin1String("foouser"));
    QCOMPARE(other->password(), QLatin1String("foopass"));
    delete other;

    // get by username
    other = users.get(QDjangoWhere("username", QDjangoWhere::Equals, "foouser"));
    QVERIFY(other != 0);
    QCOMPARE(other->pk(), QVariant(1));
    QCOMPARE(other->username(), QLatin1String("foouser"));
    QCOMPARE(other->password(), QLatin1String("foopass"));
    QCOMPARE(other->lastLogin(), QDateTime(QDate(2010, 6, 1), QTime(10, 5, 14)));
    delete other;

    // update
    user.setPassword("foopass2");
    QCOMPARE(user.save(), true);
    QCOMPARE(users.all().size(), 1);

    other = users.get(QDjangoWhere("username", QDjangoWhere::Equals, "foouser"));
    QVERIFY(other != 0);
    QCOMPARE(other->pk(), QVariant(1));
    QCOMPARE(other->username(), QLatin1String("foouser"));
    QCOMPARE(other->password(), QLatin1String("foopass2"));
    delete other;

    User user2;
    user2.setUsername("baruser");
    user2.setPassword("barpass");
    user2.setLastLogin(QDateTime(QDate(2010, 6, 1), QTime(10, 6, 31)));
    QCOMPARE(user2.save(), true);
    QCOMPARE(users.all().size(), 2);

    other = users.get(QDjangoWhere("username", QDjangoWhere::Equals, "baruser"));
    QVERIFY(other != 0);
    QCOMPARE(other->pk(), QVariant(2));
    QCOMPARE(other->username(), QLatin1String("baruser"));
    QCOMPARE(other->password(), QLatin1String("barpass"));
    QCOMPARE(other->lastLogin(), QDateTime(QDate(2010, 6, 1), QTime(10, 6, 31)));
    delete other;
}

void TestModel::removeUser()
{
    const QDjangoQuerySet<User> users;

    User user;
    user.setUsername("foouser");
    user.setPassword("foopass");

    bool result = user.save();
    QCOMPARE(result, true);
    QCOMPARE(users.all().size(), 1);

    result = user.remove();
    QCOMPARE(result, true);
    QCOMPARE(users.all().size(), 0);
}

void TestModel::removeUsers()
{
    const QDjangoQuerySet<User> users;

    User user;
    user.setUsername("foouser");
    user.setPassword("foopass");
    QCOMPARE(user.save(), true);

    User user2;
    user2.setUsername("baruser");
    user2.setPassword("nopass");
    QCOMPARE(user2.save(), true);

    User user3;
    user3.setUsername("wizuser");
    user3.setPassword("nopass");
    QCOMPARE(user3.save(), true);

    QCOMPARE(users.all().size(), 3);

    users.filter(QDjangoWhere("password", QDjangoWhere::Equals, "nopass")).remove();

    QCOMPARE(users.all().size(), 1);
}

void TestModel::getUser()
{
    const QDjangoQuerySet<User> users;

    User foo;
    foo.setUsername("foouser");
    foo.setPassword("foopass");
    foo.save();

    User bar;
    bar.setUsername("baruser");
    bar.setPassword("barpass");
    bar.save();

    QCOMPARE(users.all().size(), 2);

    User *other = users.get(QDjangoWhere("username", QDjangoWhere::Equals, "foouser"));
    QVERIFY(other != 0);
    QCOMPARE(other->username(), QLatin1String("foouser"));
    QCOMPARE(other->password(), QLatin1String("foopass"));
    delete other;
}

void TestModel::filterUsers()
{
    const QDjangoQuerySet<User> users;

    User foo;
    foo.setUsername("foouser");
    foo.setPassword("foopass");
    foo.save();

    User bar;
    bar.setUsername("baruser");
    bar.setPassword("barpass");
    bar.save();

    User wiz;
    wiz.setUsername("wizuser");
    wiz.setPassword("wizpass");
    wiz.save();

    // all users
    QDjangoQuerySet<User> qs = users.all();
    QCOMPARE(qs.where().sql(), QLatin1String(""));
    QCOMPARE(qs.size(), 3);

    // invalid username
    qs = users.filter(QDjangoWhere("username", QDjangoWhere::Equals, "doesnotexist"));
    QCOMPARE(qs.size(), 0);

    // valid username
    qs = users.filter(QDjangoWhere("username", QDjangoWhere::Equals, "foouser"));
    QCOMPARE(qs.where().sql(), QLatin1String("`user`.`username` = :user_username"));
    QCOMPARE(qs.size(), 1);
    User *other = qs.at(0);
    QVERIFY(other != 0);
    QCOMPARE(other->username(), QLatin1String("foouser"));
    QCOMPARE(other->password(), QLatin1String("foopass"));
    delete other;

    // chain filters
    qs = qs.filter(QDjangoWhere("password", QDjangoWhere::Equals, "foopass"));
    QCOMPARE(qs.where().sql(), QLatin1String("`user`.`username` = :user_username AND `user`.`password` = :user_password"));
    QCOMPARE(qs.size(), 1);

    // username in list
    qs = users.filter(QDjangoWhere("username", QDjangoWhere::IsIn, QList<QVariant>() << "foouser" << "wizuser"));
    QCOMPARE(qs.where().sql(), QLatin1String("`user`.`username` IN (:user_username_0, :user_username_1)"));
    QCOMPARE(qs.size(), 2);
}

void TestModel::excludeUsers()
{
    const QDjangoQuerySet<User> users;

    User foo;
    foo.setUsername("foouser");
    foo.setPassword("foopass");
    foo.save();

    User bar;
    bar.setUsername("baruser");
    bar.setPassword("barpass");
    bar.save();

    QDjangoQuerySet<User> qs = users.all();
    QCOMPARE(qs.where().sql(), QLatin1String(""));
    QCOMPARE(users.all().size(), 2);

    qs = users.exclude(QDjangoWhere("username", QDjangoWhere::Equals, "doesnotexist"));
    QCOMPARE(qs.where().sql(), QLatin1String("`user`.`username` != :user_username"));
    QCOMPARE(qs.size(), 2);

    qs = users.exclude(QDjangoWhere("username", QDjangoWhere::Equals, "baruser"));
    QCOMPARE(qs.where().sql(), QLatin1String("`user`.`username` != :user_username"));
    QCOMPARE(qs.size(), 1);
    User *other = qs.at(0);
    QVERIFY(other != 0);
    QCOMPARE(other->username(), QLatin1String("foouser"));
    QCOMPARE(other->password(), QLatin1String("foopass"));
    delete other;

    qs = qs.exclude(QDjangoWhere("password", QDjangoWhere::Equals, "barpass"));
    QCOMPARE(qs.where().sql(), QLatin1String("`user`.`username` != :user_username AND `user`.`password` != :user_password"));
    QCOMPARE(qs.size(), 1);
}

void TestModel::limit()
{
    const QDjangoQuerySet<User> users;

    for (int i = 0; i < 10; i++)
    {
        User user;
        user.setUsername(QString("foouser%1").arg(i));
        user.setPassword(QString("foopass%1").arg(i));
        QCOMPARE(user.save(), true);
    }

    // all results
    QDjangoQuerySet<User> qs = users.limit(0, -1);
    QCOMPARE(qs.size(), 10);

    // all results from offset 1
    qs = users.limit(1, -1);
    QCOMPARE(qs.size(), 9);
    User *other = qs.at(0);
    QCOMPARE(other->username(), QLatin1String("foouser1"));
    delete other;
    other = qs.at(8);
    QCOMPARE(other->username(), QLatin1String("foouser9"));
    delete other;

    // 5 results from offset 0
    qs = users.limit(0, 5);
    QCOMPARE(qs.size(), 5);
    other = qs.at(0);
    QCOMPARE(other->username(), QLatin1String("foouser0"));
    delete other;
    other = qs.at(4);
    QCOMPARE(other->username(), QLatin1String("foouser4"));
    delete other;

    // 6 results from offset 1
    qs = users.limit(1, 8);
    QCOMPARE(qs.size(), 8);
    other = qs.at(0);
    QCOMPARE(other->username(), QLatin1String("foouser1"));
    delete other;
    other = qs.at(7);
    QCOMPARE(other->username(), QLatin1String("foouser8"));
    delete other;
}

/** Test chaining limit statements.
 */
void TestModel::subLimit()
{
    const QDjangoQuerySet<User> users;

    for (int i = 0; i < 10; i++)
    {
        User user;
        user.setUsername(QString("foouser%1").arg(i));
        user.setPassword(QString("foopass%1").arg(i));
        QCOMPARE(user.save(), true);
    }

    // base query : 8 results from offset 1
    QDjangoQuerySet<User> refQs = users.limit(1, 8);

    // all sub-results from offset 2
    QDjangoQuerySet<User> qs = refQs.limit(2, -1);
    QCOMPARE(qs.size(), 6);
    User *other = qs.at(0);
    QCOMPARE(other->username(), QLatin1String("foouser3"));
    delete other;
    other = qs.at(5);
    QCOMPARE(other->username(), QLatin1String("foouser8"));
    delete other;

    // 4 sub-results from offset 0
    qs = refQs.limit(0, 4);
    QCOMPARE(qs.size(), 4);
    other = qs.at(0);
    QCOMPARE(other->username(), QLatin1String("foouser1"));
    delete other;
    other = qs.at(3);
    QCOMPARE(other->username(), QLatin1String("foouser4"));
    delete other;

    // 3 sub-results from offset 2
    qs = refQs.limit(2, 3);
    QCOMPARE(qs.size(), 3);
    other = qs.at(0);
    QCOMPARE(other->username(), QLatin1String("foouser3"));
    delete other;
    other = qs.at(2);
    QCOMPARE(other->username(), QLatin1String("foouser5"));
    delete other;
}

void TestModel::values()
{
    const QDjangoQuerySet<User> users;

    User user;
    user.setUsername("foouser");
    user.setPassword("foopass");
    user.setLastLogin(QDateTime(QDate(2010, 6, 1), QTime(10, 5, 14)));
    QCOMPARE(user.save(), true);

    User user2;
    user2.setUsername("baruser");
    user2.setPassword("barpass");
    user2.setLastLogin(QDateTime(QDate(2010, 6, 1), QTime(10, 6, 31)));
    QCOMPARE(user2.save(), true);

    // FIXME : test last_login
    QList< QMap<QString, QVariant> > map = users.all().values();
    QCOMPARE(map.size(), 2);
    QCOMPARE(map[0].keys(), QList<QString>() << "date_joined" << "email" << "first_name" << "id" << "is_active" << "is_staff" << "is_superuser" << "last_login" << "last_name" << "password" << "username");
    QCOMPARE(map[0]["username"], QVariant("foouser"));
    QCOMPARE(map[0]["password"], QVariant("foopass"));
    QCOMPARE(map[1].keys(), QList<QString>() << "date_joined" << "email" << "first_name" << "id" << "is_active" << "is_staff" << "is_superuser" << "last_login" << "last_name" << "password" << "username");
    QCOMPARE(map[1]["username"], QVariant("baruser"));
    QCOMPARE(map[1]["password"], QVariant("barpass"));

    // FIXME : test last_login
    map = users.all().values(QStringList() << "username" << "password");
    QCOMPARE(map.size(), 2);
    QCOMPARE(map[0].keys(), QList<QString>() << "password" << "username");
    QCOMPARE(map[0]["username"], QVariant("foouser"));
    QCOMPARE(map[0]["password"], QVariant("foopass"));
    QCOMPARE(map[1].keys(), QList<QString>() << "password" << "username");
    QCOMPARE(map[1]["username"], QVariant("baruser"));
    QCOMPARE(map[1]["password"], QVariant("barpass"));
}

void TestModel::valuesList()
{
    const QDjangoQuerySet<User> users;

    User user;
    user.setUsername("foouser");
    user.setPassword("foopass");
    user.setLastLogin(QDateTime(QDate(2010, 6, 1), QTime(10, 5, 14)));
    QCOMPARE(user.save(), true);

    User user2;
    user2.setUsername("baruser");
    user2.setPassword("barpass");
    user2.setLastLogin(QDateTime(QDate(2010, 6, 1), QTime(10, 6, 31)));
    QCOMPARE(user2.save(), true);

    // FIXME : test last_login
    QList< QList<QVariant> > list = users.all().valuesList();
    QCOMPARE(list.size(), 2);
    QCOMPARE(list[0].size(), 11);
    QCOMPARE(list[0][1], QVariant("foouser"));
    QCOMPARE(list[0][5], QVariant("foopass"));
    QCOMPARE(list[1].size(), 11);
    QCOMPARE(list[1][1], QVariant("baruser"));
    QCOMPARE(list[1][5], QVariant("barpass"));

    // FIXME : test last_login
    list = users.all().valuesList(QStringList() << "username" << "password");
    QCOMPARE(list.size(), 2);
    QCOMPARE(list[0].size(), 2);
    QCOMPARE(list[0][0], QVariant("foouser"));
    QCOMPARE(list[0][1], QVariant("foopass"));
    QCOMPARE(list[1].size(), 2);
    QCOMPARE(list[1][0], QVariant("baruser"));
    QCOMPARE(list[1][1], QVariant("barpass"));
}

void TestModel::cleanup()
{
    QDjangoQuerySet<User>().remove();
}

void TestModel::cleanupTestCase()
{
    QCOMPARE(User().dropTable(), true);
}

void TestRelated::initTestCase()
{
    QCOMPARE(User().createTable(), true);
    QCOMPARE(Group().createTable(), true);
    QCOMPARE(Message().createTable(), true);
    QCOMPARE(UserGroups().createTable(), true);
}

void TestRelated::testRelated()
{
    const QDjangoQuerySet<Message> messages;

    {
        User user;
        user.setUsername("foouser");
        user.setPassword("foopass");
        QCOMPARE(user.save(), true);

        Message message;
        message.setUserId(user.pk().toInt());
        message.setText("test message");
        QCOMPARE(message.save(), true);
    }

    // uncached
    Message *uncached = messages.get(
        QDjangoWhere("id", QDjangoWhere::Equals, "1"));
    QVERIFY(uncached != 0);

    User *uncachedUser = uncached->user();
    QVERIFY(uncachedUser != 0);
    QCOMPARE(uncachedUser->username(), QLatin1String("foouser"));
    QCOMPARE(uncachedUser->password(), QLatin1String("foopass"));
    delete uncached;

    // cached
    Message *cached = messages.selectRelated().get(
        QDjangoWhere("id", QDjangoWhere::Equals, 1));
    QVERIFY(cached != 0);

    User *cachedUser = cached->user();
    QVERIFY(cachedUser != 0);
    QCOMPARE(cachedUser->username(), QLatin1String("foouser"));
    QCOMPARE(cachedUser->password(), QLatin1String("foopass"));
    delete cached;
}

void TestRelated::filterRelated()
{
    const QDjangoQuerySet<Message> messages;
    int userPk;
    {
        User user;
        user.setUsername("foouser");
        user.setPassword("foopass");
        QCOMPARE(user.save(), true);
        userPk = user.pk().toInt();

        Message message;
        message.setUserId(userPk);
        message.setText("test message");
        QCOMPARE(message.save(), true);
    }

    // FIXME : does not work without selectRelated
    QDjangoQuerySet<Message> qs = messages.selectRelated().filter(
        QDjangoWhere("user__username", QDjangoWhere::Equals, "foouser"));
    QCOMPARE(qs.where().sql(), QLatin1String("`user`.`username` = :user_username"));
    QCOMPARE(qs.size(), 1);

    Message *msg = qs.at(0);
    QVERIFY(msg != 0);
    QCOMPARE(msg->text(), QLatin1String("test message"));
    QCOMPARE(msg->userId(), userPk);
    delete msg;
}

void TestRelated::testGroups()
{
    const QDjangoQuerySet<UserGroups> userGroups;

    User user;
    user.setUsername("foouser");
    user.setPassword("foopass");
    QCOMPARE(user.save(), true);

    Group group;
    group.setName("foogroup");
    QCOMPARE(group.save(), true);

    UserGroups userGroup;
    userGroup.setUserId(user.pk().toInt());
    userGroup.setGroupId(group.pk().toInt());
    QCOMPARE(userGroup.save(), true);
    
    UserGroups *ug = userGroups.selectRelated().get(
        QDjangoWhere("id", QDjangoWhere::Equals, 1));
    QVERIFY(ug != 0);
    delete ug;
}

void TestRelated::cleanup()
{
    QDjangoQuerySet<User>().remove();
    QDjangoQuerySet<Group>().remove();
    QDjangoQuerySet<Message>().remove();
    QDjangoQuerySet<UserGroups>().remove();
}

void TestRelated::cleanupTestCase()
{
    QCOMPARE(User().dropTable(), true);
    QCOMPARE(Group().dropTable(), true);
    QCOMPARE(Message().dropTable(), true);
    QCOMPARE(UserGroups().dropTable(), true);
}
