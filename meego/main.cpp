/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
// Einstiegspunkt der MeeGo-Harmattan-Ausgabe (Nokia N9). Qt 4.7 mit
// QtQuick 1.1: derselbe Regelkern und dieselbe SeucheEngine wie auf Sailfish,
// nur eine eigene Oberfläche unter meego/qml.
#include <QApplication>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeError>
#include <QDeclarativeView>
#include <QDir>
#include <QTextCodec>
#include <QUrl>
#include <QtDeclarative>

#include <cstdio>

#include "SeucheEngine.h"
#include "LinkItem.h"
#include "Theme.h"

// Der N9 hat kein gdb und wirft Core-Dateien weg: lieber eine rohe
// Rückverfolgung ausgeben, die addr2line auf dem Baurechner gegen das
// ungestrippte build/meego/arm/harbour-seuche auflösen kann.
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>

static void crashHandler(int sig)
{
    void* frames[64];
    const int n = backtrace(frames, 64);
    const char head[] = "harbour-seuche: fatal signal, backtrace:\n";
    if (write(2, head, sizeof(head) - 1) < 0) {}
    backtrace_symbols_fd(frames, n, 2);
    signal(sig, SIG_DFL);
    raise(sig);
}

int main(int argc, char* argv[])
{
    signal(SIGSEGV, crashHandler);
    signal(SIGABRT, crashHandler);
    signal(SIGBUS, crashHandler);
    signal(SIGILL, crashHandler);

    QApplication app(argc, argv);
    // Qt 4 schiebt unübersetzte tr()-Quelltexte durch Latin-1; die deutschen
    // Texte hier sind UTF-8.
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QCoreApplication::setOrganizationName(QString::fromLatin1("harbour-seuche"));
    QCoreApplication::setApplicationName(QString::fromLatin1("harbour-seuche"));

    qmlRegisterType<LinkItem>("harbour.seuche", 1, 0, "LinkItem");

    // Installiert als /opt/harbour-seuche/{bin,qml,icons}; SEUCHE_ROOT
    // überschreibt das für Läufe auf dem Schreibtisch.
    QString root = QString::fromLocal8Bit(qgetenv("SEUCHE_ROOT"));
    if (root.isEmpty())
        root = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(QString::fromLatin1(".."));

    // Vor der Ansicht angelegt, damit sie jede Bindung überleben.
    SeucheEngine engine;
    MeegoTheme theme;
    MeegoStyle style;

    QDeclarativeView view;
    view.setResizeMode(QDeclarativeView::SizeRootObjectToView);
    view.rootContext()->setContextProperty(QString::fromLatin1("engine"), &engine);
    // NICHT "Theme": com.nokia.meego exportiert selbst ein Theme, und
    // das gewinnt gegen jede gleichnamige Kontext-Eigenschaft. Die
    // QML bekommt dann stillschweigend lauter undefined-Werte --
    // Schriftgroessen, Farben und Abstaende fehlen, und die Seite
    // sieht aus, als waere sie nie gestaltet worden.
    view.rootContext()->setContextProperty(QString::fromLatin1("AppTheme"), &theme);
    view.rootContext()->setContextProperty(QString::fromLatin1("Style"), &style);
    view.setSource(QUrl::fromLocalFile(root + QString::fromLatin1("/qml/harbour-seuche.qml")));
    if (view.status() == QDeclarativeView::Error) {
        const QList<QDeclarativeError> errors = view.errors();
        for (int i = 0; i < errors.size(); ++i)
            std::fprintf(stderr, "%s\n", qPrintable(errors[i].toString()));
        return 1;
    }

    // Der N9 fährt Anwendungen formatfüllend; ein Lauf am Schreibtisch
    // bekommt ein Fenster in der Hochformatgröße des Geräts.
    if (qgetenv("SEUCHE_WINDOWED").isEmpty()) {
        view.showFullScreen();
    } else {
        view.resize(480, 854);
        view.setVisible(true);
    }
    return app.exec();
}
