// Die angefangene Partie überlebt das Schließen der App.
//
// Geprüft wird nicht, dass eine Datei entsteht, sondern dass ein **zweites,
// frisch gebautes Triebwerk** dieselbe Partie vor sich hat: dieselbe Stellung,
// dieselben Hände, dieselbe Reihenfolge der beiden verdeckten Stapel und
// derselbe Zufall. Die beiden letzten sind der eigentliche Punkt — ein
// Spielstand, der nur die sichtbare Lage sichert, teilt beim Weiterspielen
// leere Karten aus.
#include "SeucheEngine.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QVariant>

#include <cstdio>

static int failures = 0;

static void check(bool ok, const QString &what)
{
    if (!ok) {
        std::printf("FAIL  %s\n", qPrintable(what));
        ++failures;
    }
}

// Die Lage, wie die Oberfläche sie sieht — alles, was sich zwischen zwei
// Triebwerken unterscheiden könnte.
static QVariantMap facade(SeucheEngine &engine)
{
    QVariantMap map;
    map[QStringLiteral("running")] = engine.running();
    map[QStringLiteral("phase")] = engine.phase();
    map[QStringLiteral("seats")] = engine.seats();
    map[QStringLiteral("atTurn")] = engine.atTurn();
    map[QStringLiteral("actionsLeft")] = engine.actionsLeft();
    map[QStringLiteral("drawsLeft")] = engine.drawsLeft();
    map[QStringLiteral("infectionsLeft")] = engine.infectionsLeft();
    map[QStringLiteral("rate")] = engine.infectionRate();
    map[QStringLiteral("outbreaks")] = engine.outbreaks();
    map[QStringLiteral("stations")] = engine.stations();
    map[QStringLiteral("playerDeck")] = engine.playerDeck();
    map[QStringLiteral("infectionDeck")] = engine.infectionDeck();
    map[QStringLiteral("cities")] = engine.cities();
    map[QStringLiteral("players")] = engine.players();
    map[QStringLiteral("cures")] = engine.cures();
    return map;
}

// Ein paar Aktionen, damit der Stand nicht der Startstand ist.
static void playABit(SeucheEngine &engine, int actions)
{
    for (int i = 0; i < actions; ++i) {
        const QVariantList legal = engine.allActions();
        if (legal.isEmpty())
            break;
        if (!engine.run(legal[0].toMap().value(QStringLiteral("index")).toInt()))
            break;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    const QString directory = QDir::tempPath() + QStringLiteral("/seuche-save-test");
    QDir(directory).removeRecursively();
    qputenv("SEUCHE_DATA_DIR", directory.toLocal8Bit());
    const QString path = directory + QStringLiteral("/partie.json");

    // --- nichts da, nichts zu holen ---------------------------------------
    {
        SeucheEngine leer;
        check(!leer.running(), QStringLiteral("ohne Spielstand laeuft nichts"));
    }

    QVariantMap before;
    {
        SeucheEngine engine;
        engine.newGame(3, 1);
        check(engine.running(), QStringLiteral("die Partie laeuft"));
        playABit(engine, 3);
        check(QFile::exists(path), QStringLiteral("der Stand liegt auf der Platte"));
        before = facade(engine);
    }

    // --- ein zweites Triebwerk ist derselbe Start der App -------------------
    SeucheEngine wieder;
    check(wieder.running(), QStringLiteral("die angefangene Partie ist wieder da"));
    check(!wieder.over(), QStringLiteral("und sie ist nicht zu Ende"));
    const QVariantMap after = facade(wieder);
    for (const QString &key : before.keys()) {
        check(before.value(key) == after.value(key),
              QStringLiteral("gleich geblieben: %1").arg(key));
    }

    // --- die Reihenfolge der Stapel und der Zufall --------------------------
    //
    // Zwei Triebwerke aus demselben Stand müssen dieselbe Karte ziehen und
    // dieselbe Stadt infizieren. Ohne die Stapelreihenfolge im Spielstand
    // käme hier eine leere Karte, ohne den Zufall spätestens bei der nächsten
    // Epidemie eine andere Mischung.
    SeucheEngine zwilling;
    check(zwilling.running(), QStringLiteral("auch das zweite liest denselben Stand"));
    for (int step = 0; step < 12; ++step) {
        const QString phase = wieder.phase();
        if (phase == QLatin1String("actions")) {
            const QVariantList legal = wieder.allActions();
            const QVariantList same = zwilling.allActions();
            check(legal.size() == same.size(),
                  QStringLiteral("gleich viele Zuege (Schritt %1)").arg(step));
            if (legal.isEmpty())
                break;
            const int index = legal[0].toMap().value(QStringLiteral("index")).toInt();
            wieder.run(index);
            zwilling.run(index);
        } else if (phase == QLatin1String("draw")) {
            wieder.drawCard();
            zwilling.drawCard();
        } else if (phase == QLatin1String("infect")) {
            wieder.infectCity();
            zwilling.infectCity();
        } else if (phase == QLatin1String("discard")) {
            const int seat = wieder.discardingSeat();
            const QVariantList hand = wieder.players()[seat].toMap()
                                          .value(QStringLiteral("hand")).toList();
            if (hand.isEmpty())
                break;
            const int card = hand[0].toMap().value(QStringLiteral("id")).toInt();
            wieder.discardCard(card);
            zwilling.discardCard(card);
        } else {
            break;
        }
        check(facade(wieder) == facade(zwilling),
              QStringLiteral("beide laufen gleich weiter (Schritt %1)").arg(step));
    }

    // --- wegwerfen ----------------------------------------------------------
    wieder.discardSavedGame();
    check(!QFile::exists(path), QStringLiteral("der Stand ist weg"));
    check(!wieder.running(), QStringLiteral("und die Partie auch"));
    {
        SeucheEngine danach;
        check(!danach.running(), QStringLiteral("nach dem Wegwerfen faengt nichts wieder an"));
    }

    // --- eine Netzpartie fasst den eigenen Stand nicht an -------------------
    {
        SeucheEngine eigen;
        eigen.newGame(2, 0);
        playABit(eigen, 2);
        check(QFile::exists(path), QStringLiteral("eigene Partie wieder gesichert"));
    }
    {
        SeucheEngine gast;
        check(gast.running(), QStringLiteral("die eigene Partie ist da"));
        // joinGame ohne Gegenstelle: die Verbindung kommt nie zustande, aber
        // das Geraet ist ab jetzt kein Alleinspieler mehr.
        gast.joinGame(QStringLiteral("127.0.0.1"));
        check(QFile::exists(path),
              QStringLiteral("der eigene Stand bleibt liegen, wenn im Netz gespielt wird"));
        gast.leaveNetwork();
        check(QFile::exists(path), QStringLiteral("und auch danach noch"));
    }
    {
        SeucheEngine zurueck;
        check(zurueck.running(), QStringLiteral("die eigene Partie ist immer noch da"));
    }

    QDir(directory).removeRecursively();
    std::printf(failures == 0 ? "all save checks passed\n" : "%d save checks failed\n", failures);
    return failures == 0 ? 0 : 1;
}
