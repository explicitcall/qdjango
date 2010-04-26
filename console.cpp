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

#include <getopt.h>
#include <cstdlib>

#include <QDebug>
#include <QStringList>
#include <QCoreApplication>
#include <QFileInfo>
#include <QScriptEngine>
#include <QSettings>
#include <QSqlDatabase>
#include <QTextStream>
#include <QtPlugin>

#include "model.h"

static bool wantsToQuit;

static QScriptValue qtscript_load(QScriptContext *ctx, QScriptEngine *eng)
{
    QString name = ctx->argument(0).toString();
    eng->importExtension(name);
    return eng->undefinedValue();
}

static QScriptValue qtscript_quit(QScriptContext *ctx, QScriptEngine *eng)
{
    Q_UNUSED(ctx);
    wantsToQuit = true;
    return eng->undefinedValue();
}

static void interactive(QScriptEngine *eng)
{
    QScriptValue global = eng->globalObject();
    QScriptValue loadFunction = eng->newFunction(qtscript_load);
    QScriptValue quitFunction = eng->newFunction(qtscript_quit);
    if (!global.property(QLatin1String("load")).isValid())
        global.setProperty(QLatin1String("load"), loadFunction);
    if (!global.property(QLatin1String("quit")).isValid())
        global.setProperty(QLatin1String("quit"), quitFunction);
    wantsToQuit = false;

    QTextStream qin(stdin, QIODevice::ReadOnly);

    const char *qscript_prompt = "qdjango> ";
    const char *dot_prompt = ".... ";
    const char *prompt = qscript_prompt;

    QString code;

    printf("Commands:\n"
           "\tquit() : exits console \n"
           "\tload() : loads a QtScript extension\n");

    forever {
        QString line;

        printf("%s", prompt);
        fflush(stdout);

        line = qin.readLine();
        if (line.isNull())
            break;

        code += line;
        code += QLatin1Char('\n');

        if (line.trimmed().isEmpty()) {
            continue;

        } else if (! eng->canEvaluate(code)) {
            prompt = dot_prompt;

        } else {
            QScriptValue result = eng->evaluate(code, QLatin1String("typein"));

            code.clear();
            prompt = qscript_prompt;

            if (! result.isUndefined())
                fprintf(stderr, "%s\n", qPrintable(result.toString()));

            if (wantsToQuit)
                break;
        }
    }

}

int main(int argc, char *argv[])
{
    int c;
    QString databaseName(":memory:");

    /* Create application */
    QCoreApplication app(argc, argv);

    /* Parse command line arguments */
    while ((c = getopt(argc, argv, "d:hp:")) != -1)
    {
        if (c == 'h')
        {
            fprintf(stderr, "Usage: qdjango-console -p <plugins_path> -d <database_name>\n");
            return EXIT_SUCCESS;
        } else if (c == 'd') {
            databaseName = QString::fromLocal8Bit(optarg);
        } else if (c == 'p') {
            app.setLibraryPaths(app.libraryPaths() << QString::fromLocal8Bit(optarg)); 
        }
    }

    /* Open database */
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(databaseName);
    if (!db.open())
    {
        fprintf(stderr, "Could not access database\n");
        return EXIT_FAILURE;
    }
    QDjangoModel::setDatabase(&db);

    /* Run interactive shell */ 
    QScriptEngine *eng = new QScriptEngine();
    qDebug() << "Available extensions: " << eng->availableExtensions();
    interactive(eng);

    return EXIT_SUCCESS;
}

