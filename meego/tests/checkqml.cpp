/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
// Übersetzt jede QML-Datei der N9-Oberfläche mit dem Qt 4.7 des Geräts und
// meldet, was dabei schiefgeht: unbekannte Typen, unbekannte Eigenschaften,
// Schreibfehler in Bezeichnern — alles, was qmllint nicht sieht, weil es nur
// die Grammatik prüft, und was sonst erst auf dem Telefon auffiele.
//
// Übersetzen ist nicht ausführen: ob die Bindungen sinnvolle Werte liefern und
// ob das Bild stimmt, zeigt erst ein Lauf. Aber ein `readonly property`, das
// es in QtQuick 1.1 nicht gibt, oder ein Silica-Rest fällt hier auf.
//
// Der Baurechner hat keinen X-Server, deshalb QCoreApplication. Das Übersetzen
// einer Komponente braucht keinen Bildschirm — wohl aber das Plugin von
// com.nokia.meego, das beim Laden ein QWidget anlegt und ohne GUI nicht etwa
// einen Fehler meldet, sondern den Prozess abbricht. Dateien, die es
// importieren, werden deshalb gar nicht erst angefasst und als übersprungen
// gemeldet. Wirklich geprüft werden so die eigenen Bausteine und Board.qml,
// wo die meiste Rechnerei steckt; für den Rest gibt es tests/lint-qml.py.
#include <QCoreApplication>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeError>
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QUrl>
#include <QtDeclarative>

#include <cstdio>

#include "LinkItem.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    const QStringList args = app.arguments();
    if (args.size() < 3) {
        std::fprintf(stderr, "usage: checkqml <qml-dir> <import-dir>\n");
        return 2;
    }
    const QString qmlDir = args.at(1);
    const QString importDir = args.at(2);

    // Denselben Typ anmelden, den meego/main.cpp anmeldet, sonst kennt
    // Board.qml sein LinkItem nicht.
    qmlRegisterType<LinkItem>("harbour.seuche", 1, 0, "LinkItem");

    QDeclarativeEngine engine;
    engine.addImportPath(importDir);
    engine.addImportPath(qmlDir);

    // Die Kontexteigenschaften, die meego/main.cpp setzt. Hier reichen leere
    // Platzhalter: geprüft werden Typen und Eigenschaften, nicht Werte.
    QObject placeholder;
    engine.rootContext()->setContextProperty(QLatin1String("engine"), &placeholder);
    engine.rootContext()->setContextProperty(QLatin1String("Theme"), &placeholder);
    engine.rootContext()->setContextProperty(QLatin1String("Style"), &placeholder);

    QDir dir(qmlDir);
    QStringList files = dir.entryList(QStringList() << QLatin1String("*.qml"), QDir::Files);
    files.sort();

    int bad = 0;
    int skipped = 0;
    int checked = 0;
    (void)importDir;
    for (int i = 0; i < files.size(); ++i) {
        const QString path = dir.filePath(files.at(i));

        QFile source(path);
        if (source.open(QIODevice::ReadOnly)) {
            const QByteArray text = source.readAll();
            source.close();
            if (text.contains("import com.nokia.meego")) {
                std::printf("übersprungen (braucht einen Bildschirm) %s\n",
                            qPrintable(files.at(i)));
                ++skipped;
                continue;
            }
        }

        QDeclarativeComponent component(&engine, QUrl::fromLocalFile(path));
        if (!component.isError()) {
            ++checked;
            continue;
        }

        const QList<QDeclarativeError> errors = component.errors();
        bool needsDisplay = false;
        for (int e = 0; e < errors.size(); ++e) {
            if (errors.at(e).toString().contains(
                    QLatin1String("plugin cannot be loaded for module")))
                needsDisplay = true;
        }
        if (needsDisplay) {
            std::printf("übersprungen (braucht einen Bildschirm) %s\n", qPrintable(files.at(i)));
            ++skipped;
            continue;
        }

        std::printf("FEHLER %s\n", qPrintable(files.at(i)));
        for (int e = 0; e < errors.size(); ++e)
            std::printf("       %s\n", qPrintable(errors.at(e).toString()));
        ++bad;
    }

    std::printf("%d QML-Dateien: %d übersetzt, %d übersprungen, %d fehlerhaft\n",
                files.size(), checked, skipped, bad);
    return bad == 0 ? 0 : 1;
}
