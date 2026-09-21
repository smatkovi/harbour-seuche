// The facade seen from QML. Every property and method the pages read is
// evaluated in a real QQmlEngine: a typo or a missing metatype shows up here as
// `undefined` instead of as a silently empty page on the device.
//
// The second half drives a whole game through the same calls the pages make, so
// the facade is known to carry a match from the first action to the end.
#include "SeucheEngine.h"

#include <QCoreApplication>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
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

// Reads one expression against the engine and reports whether it is defined.
static QVariant evaluate(QQmlEngine *qml, const QString &expression)
{
    QQmlComponent component(qml);
    component.setData(QStringLiteral("import QtQml 2.0\n"
                                     "QtObject { property var value: %1 }\n")
                          .arg(expression).toUtf8(),
                      QUrl());
    QObject *object = component.create();
    if (!object) {
        std::printf("FAIL  %s does not even compile: %s\n", qPrintable(expression),
                    qPrintable(component.errorString()));
        ++failures;
        return QVariant();
    }
    const QVariant value = object->property("value");
    delete object;
    return value;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    SeucheEngine engine;
    QQmlEngine qml;
    qml.rootContext()->setContextProperty(QStringLiteral("engine"), &engine);

    // before a game starts nothing may throw
    const QStringList always = {
        QStringLiteral("engine.running"), QStringLiteral("engine.over"),
        QStringLiteral("engine.phase"), QStringLiteral("engine.phaseText"),
        QStringLiteral("engine.cities"), QStringLiteral("engine.links"),
        QStringLiteral("engine.players"), QStringLiteral("engine.cures"),
        QStringLiteral("engine.journal"),
    };
    for (const QString &expression : always)
        check(evaluate(&qml, expression).isValid(), expression + QStringLiteral(" is undefined"));

    engine.newGame(4, 1);

    // every property and call the pages use
    const QStringList used = {
        QStringLiteral("engine.running"), QStringLiteral("engine.over"),
        QStringLiteral("engine.phase"), QStringLiteral("engine.phaseText"),
        QStringLiteral("engine.outcomeText"), QStringLiteral("engine.seats"),
        QStringLiteral("engine.atTurn"), QStringLiteral("engine.actionsLeft"),
        QStringLiteral("engine.drawsLeft"), QStringLiteral("engine.infectionsLeft"),
        QStringLiteral("engine.infectionRate"), QStringLiteral("engine.outbreaks"),
        QStringLiteral("engine.stations"), QStringLiteral("engine.playerDeck"),
        QStringLiteral("engine.infectionDeck"), QStringLiteral("engine.discardingSeat"),
        QStringLiteral("engine.cities"), QStringLiteral("engine.links"),
        QStringLiteral("engine.players"), QStringLiteral("engine.cures"),
        QStringLiteral("engine.journal"),
        QStringLiteral("engine.allActions()"),
        QStringLiteral("engine.actionsForCity(2)"),
        QStringLiteral("engine.eventCards()"),
        QStringLiteral("engine.infectionDiscard()"),
        QStringLiteral("engine.forecastCards()"),
        QStringLiteral("engine.cityName(2)"),
        QStringLiteral("engine.cardLabel(2)"),
    };
    for (const QString &expression : used)
        check(evaluate(&qml, expression).isValid(), expression + QStringLiteral(" is undefined"));

    // the fields the delegates read off those lists
    check(evaluate(&qml, QStringLiteral("engine.cities.length")).toInt() == 48, "48 cities in QML");
    check(evaluate(&qml, QStringLiteral("engine.links.length")).toInt() == 93, "93 links in QML");
    check(evaluate(&qml, QStringLiteral("engine.cities[2].name")).toString() == QStringLiteral("Atlanta"),
          "city 2 is Atlanta");
    check(evaluate(&qml, QStringLiteral("engine.cities[2].station")).toBool(), "Atlanta has the station");
    check(evaluate(&qml, QStringLiteral("engine.cities[2].cubes.length")).toInt() == 4, "four cube counts");
    check(evaluate(&qml, QStringLiteral("engine.cities[2].pawns.length")).toInt() == 4, "four pawns in Atlanta");
    check(evaluate(&qml, QStringLiteral("engine.players[0].hand.length")).toInt() == 2, "two cards at four seats");
    check(!evaluate(&qml, QStringLiteral("engine.players[0].role")).toString().isEmpty(), "role has a name");
    check(!evaluate(&qml, QStringLiteral("engine.players[0].ability")).toString().isEmpty(), "ability has a text");
    check(!evaluate(&qml, QStringLiteral("engine.cures[0].name")).toString().isEmpty(), "cure has a name");
    check(evaluate(&qml, QStringLiteral("engine.allActions().length")).toInt() > 0, "actions are offered");
    check(!evaluate(&qml, QStringLiteral("engine.allActions()[0].label")).toString().isEmpty(),
          "actions carry a label");
    check(evaluate(&qml, QStringLiteral("engine.links[0].x1")).isValid(), "links carry coordinates");

    // --- roles: free, but never twice -------------------------------------
    check(evaluate(&qml, QStringLiteral("engine.rolesLocked")).isValid(),
          QStringLiteral("engine.rolesLocked is undefined"));
    check(evaluate(&qml, QStringLiteral("engine.freeRoles(0)")).isValid(),
          QStringLiteral("engine.freeRoles() is undefined"));
    check(!engine.rolesLocked(), QStringLiteral("roles are open before the first move"));

    {
        // four seats hold four of the seven roles, so three are gone for each
        const QVariantList offered = engine.freeRoles(0);
        check(offered.size() == seuche::kBaseRoles - 3, QStringLiteral("only free roles are offered"));

        const auto roleNumber = [](const QString &name) {
            for (int role = 0; role < seuche::kBaseRoles; ++role) {
                if (QString::fromUtf8(seuche::roleName(seuche::Role(role))) == name)
                    return role;
            }
            return -1;
        };

        const int neighbour = roleNumber(engine.players()[1].toMap()
                                             .value(QStringLiteral("role")).toString());
        check(neighbour >= 0, QStringLiteral("the neighbour's role has a name"));
        check(!engine.chooseRole(0, neighbour), QStringLiteral("a taken role is refused"));
        for (const QVariant &entry : offered) {
            check(entry.toMap().value(QStringLiteral("roleId")).toInt() != neighbour,
                  QStringLiteral("a taken role is not even offered"));
        }

        // a free one goes through and leaves the other seats' lists
        const int wanted = offered.last().toMap().value(QStringLiteral("roleId")).toInt();
        check(engine.chooseRole(0, wanted), QStringLiteral("a free role can be chosen"));
        check(roleNumber(engine.players()[0].toMap().value(QStringLiteral("role")).toString()) == wanted,
              QStringLiteral("the seat plays the chosen role"));
        for (const QVariant &entry : engine.freeRoles(1)) {
            check(entry.toMap().value(QStringLiteral("roleId")).toInt() != wanted,
                  QStringLiteral("a chosen role is off the list everywhere"));
        }
    }

    // --- undo, and the question after the fourth action -------------------
    check(evaluate(&qml, QStringLiteral("engine.canUndo")).isValid(),
          QStringLiteral("engine.canUndo is undefined"));
    check(evaluate(&qml, QStringLiteral("engine.undoText")).isValid(),
          QStringLiteral("engine.undoText is undefined"));
    check(evaluate(&qml, QStringLiteral("engine.awaitingTurnEnd")).isValid(),
          QStringLiteral("engine.awaitingTurnEnd is undefined"));
    check(evaluate(&qml, QStringLiteral("engine.players[0].roleId")).isValid(),
          QStringLiteral("players carry a roleId, which is what colours the pawn"));

    {
        const auto runFirst = [&]() {
            const QVariantList actions = engine.allActions();
            if (actions.isEmpty())
                return false;
            return engine.run(actions[0].toMap().value(QStringLiteral("index")).toInt());
        };

        check(!engine.canUndo(), QStringLiteral("nothing to take back before the first move"));
        check(engine.undoText().isEmpty(), QStringLiteral("and nothing to name either"));

        const int before = engine.actionsLeft();
        const int seat = engine.atTurn();
        const int city = engine.players()[seat].toMap().value(QStringLiteral("city")).toInt();
        check(runFirst(), QStringLiteral("the first action runs"));
        check(engine.canUndo(), QStringLiteral("an action can be taken back"));
        check(!engine.undoText().isEmpty(), QStringLiteral("undo names the move it would undo"));
        check(engine.actionsLeft() == before - 1, QStringLiteral("the action was spent"));
        check(engine.undo(), QStringLiteral("undo runs"));
        check(engine.actionsLeft() == before, QStringLiteral("the action came back"));
        check(engine.atTurn() == seat, QStringLiteral("the same seat is at turn again"));
        check(engine.players()[seat].toMap().value(QStringLiteral("city")).toInt() == city,
              QStringLiteral("the pawn stands where it stood"));
        check(!engine.canUndo(), QStringLiteral("one action taken back, nothing left"));

        // Play the turn out. The action phase ends by itself, but nothing has
        // been turned face up, so the whole turn is still reversible -- that is
        // what lets the board page ask whether the turn is really over.
        for (int i = 0; i < seuche::kActionsPerTurn; ++i) {
            if (engine.phase() != QLatin1String("actions"))
                break;
            check(runFirst(), QStringLiteral("action %1 of the turn runs").arg(i + 1));
        }
        if (engine.phase() == QLatin1String("draw")) {
            check(engine.awaitingTurnEnd(),
                  QStringLiteral("after the fourth action the page asks whether the turn is over"));
            check(engine.canUndo(), QStringLiteral("the fourth action is still reversible"));
            check(engine.undo(), QStringLiteral("answering no takes the action back"));
            check(engine.phase() == QLatin1String("actions"),
                  QStringLiteral("and puts the turn back into the action phase"));
            check(engine.actionsLeft() == 1, QStringLiteral("with the fourth action in hand again"));
            check(!engine.awaitingTurnEnd(), QStringLiteral("the question is gone"));

            // Play it again and draw: the card is seen, and that is final.
            check(runFirst(), QStringLiteral("the fourth action runs a second time"));
            engine.drawCard();
            check(!engine.canUndo(), QStringLiteral("a drawn card closes the undo window"));
            check(!engine.awaitingTurnEnd(), QStringLiteral("and the question with it"));
        }
    }

    // a whole game through the very calls the pages make
    int steps = 0;
    while (!engine.over() && steps++ < 20000) {
        const QString phase = engine.phase();
        if (phase == QLatin1String("actions")) {
            const QVariantList actions = engine.allActions();
            check(!actions.isEmpty(), QStringLiteral("there is always an action"));
            if (actions.isEmpty())
                break;
            const int index = actions[steps % actions.size()].toMap()
                                  .value(QStringLiteral("index")).toInt();
            check(engine.run(index), QStringLiteral("an offered action runs"));
        } else if (phase == QLatin1String("draw")) {
            engine.drawCard();
        } else if (phase == QLatin1String("discard")) {
            const int seat = engine.discardingSeat();
            const QVariantList hand = engine.players()[seat].toMap()
                                          .value(QStringLiteral("hand")).toList();
            check(!hand.isEmpty(), QStringLiteral("the discarding seat has cards"));
            check(engine.discardCard(hand[0].toMap().value(QStringLiteral("id")).toInt()),
                  QStringLiteral("discarding works"));
        } else if (phase == QLatin1String("infect")) {
            engine.infectCity();
        } else {
            break;
        }
    }
    check(engine.rolesLocked(), QStringLiteral("roles lock once the game runs"));
    check(!engine.chooseRole(0, 0), QStringLiteral("no role change in a running game"));
    check(engine.over(), QStringLiteral("the game reached an end"));
    check(!engine.outcomeText().isEmpty(), QStringLiteral("the end has a text"));
    check(!engine.journal().isEmpty(), QStringLiteral("the journal recorded it"));

    std::printf(failures == 0 ? "all qml bridge checks passed\n" : "%d qml bridge checks failed\n",
                failures);
    return failures == 0 ? 0 : 1;
}
