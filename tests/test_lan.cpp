// A LAN game in one process: one host and two guests over real TCP on the
// loopback. The point is not the transport (that one is shared with
// harbour-snapszer) but the rule of the mode: the host is the only device that
// changes the game, guests see exactly the same board, and a device may only
// move the seat it was given.
#include "SeucheEngine.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QTimer>

#include <functional>

#include <cstdio>

static int failures = 0;

static void check(bool ok, const QString &what)
{
    if (!ok) {
        std::printf("FAIL  %s\n", qPrintable(what));
        ++failures;
    }
}

// Runs the event loop for a while, so sockets get their turn.
static void pump(int milliseconds)
{
    QEventLoop loop;
    QTimer::singleShot(milliseconds, &loop, &QEventLoop::quit);
    loop.exec();
}

// Waits until `ready` holds, or gives up.
static bool waitFor(std::function<bool()> ready, int milliseconds = 4000)
{
    QElapsedTimer timer;
    timer.start();
    while (!ready() && timer.elapsed() < milliseconds)
        pump(25);
    return ready();
}

// The board as the device sees it: enough to notice any divergence.
static QString fingerprint(const SeucheEngine &engine)
{
    QString text = engine.phase() + QStringLiteral("|%1|%2|%3|%4|%5")
        .arg(engine.atTurn()).arg(engine.actionsLeft()).arg(engine.outbreaks())
        .arg(engine.playerDeck()).arg(engine.infectionDeck());
    const QVariantList cities = engine.cities();
    for (const QVariant &value : cities) {
        const QVariantMap city = value.toMap();
        for (const QVariant &cubes : city.value(QStringLiteral("cubes")).toList())
            text += QString::number(cubes.toInt());
        text += city.value(QStringLiteral("station")).toBool() ? QStringLiteral("S") : QStringLiteral(".");
    }
    const QVariantList players = engine.players();
    for (const QVariant &value : players) {
        const QVariantMap player = value.toMap();
        text += QStringLiteral("|") + player.value(QStringLiteral("cityName")).toString();
        for (const QVariant &card : player.value(QStringLiteral("hand")).toList())
            text += QStringLiteral(",") + card.toMap().value(QStringLiteral("label")).toString();
    }
    return text;
}

// One step on whichever device is allowed to take it.
static bool step(SeucheEngine *engine)
{
    const QString phase = engine->phase();
    if (phase == QLatin1String("actions")) {
        const QVariantList actions = engine->allActions();
        if (actions.isEmpty())
            return false;
        return engine->run(actions[actions.size() / 2].toMap()
                               .value(QStringLiteral("index")).toInt());
    }
    if (phase == QLatin1String("draw")) {
        engine->drawCard();
        return true;
    }
    if (phase == QLatin1String("infect")) {
        engine->infectCity();
        return true;
    }
    if (phase == QLatin1String("discard")) {
        const int seat = engine->discardingSeat();
        const QVariantList hand = engine->players()[seat].toMap()
                                      .value(QStringLiteral("hand")).toList();
        if (hand.isEmpty())
            return false;
        return engine->discardCard(hand[0].toMap().value(QStringLiteral("id")).toInt());
    }
    return false;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    SeucheEngine host;
    SeucheEngine guestA;
    SeucheEngine guestB;

    check(host.hostGame(3, 0, QStringLiteral("Testtisch")), QStringLiteral("hosting starts"));
    check(host.netRole() == QLatin1String("host"), QStringLiteral("host knows its role"));
    check(host.running(), QStringLiteral("the host dealt a game"));

    guestA.joinGame(QStringLiteral("127.0.0.1"));
    check(waitFor([&] { return guestA.running() && !guestA.mySeats().isEmpty(); }),
          QStringLiteral("the first guest gets a state and a seat"));
    guestB.joinGame(QStringLiteral("127.0.0.1"));
    check(waitFor([&] { return guestB.running() && !guestB.mySeats().isEmpty(); }),
          QStringLiteral("the second guest gets a state and a seat"));

    check(guestA.netRole() == QLatin1String("guest"), QStringLiteral("guest knows its role"));
    check(host.peerCount() == 2, QStringLiteral("two devices connected"));
    check(guestA.mySeats() != guestB.mySeats(), QStringLiteral("the guests hold different seats"));
    check(host.mySeats().size() == 1, QStringLiteral("the host keeps one seat"));
    check(fingerprint(host) == fingerprint(guestA), QStringLiteral("guest A sees the host's board"));
    check(fingerprint(host) == fingerprint(guestB), QStringLiteral("guest B sees the host's board"));

    // A guest may not move a seat it does not hold.
    SeucheEngine *notAtTurn = guestA.mayAct() ? &guestB : &guestA;
    const QString before = fingerprint(host);
    const QVariantList foreign = notAtTurn->allActions();
    if (!foreign.isEmpty())
        notAtTurn->run(foreign[0].toMap().value(QStringLiteral("index")).toInt());
    pump(200);
    check(fingerprint(host) == before, QStringLiteral("a foreign seat cannot be moved"));

    // Play the game out across the wire.
    SeucheEngine *devices[] = {&host, &guestA, &guestB};
    int steps = 0;
    while (!host.over() && steps++ < 4000) {
        SeucheEngine *acting = nullptr;
        for (SeucheEngine *device : devices) {
            if (device->mayAct()) {
                acting = device;
                break;
            }
        }
        check(acting != nullptr, QStringLiteral("someone is always allowed to act"));
        if (!acting)
            break;
        if (!step(acting))
            break;
        pump(acting == &host ? 1 : 30);       // guests need a round trip
    }

    check(host.over(), QStringLiteral("the game ended"));
    check(waitFor([&] { return fingerprint(guestA) == fingerprint(host); }),
          QStringLiteral("guest A ends on the same board"));
    check(waitFor([&] { return fingerprint(guestB) == fingerprint(host); }),
          QStringLiteral("guest B ends on the same board"));
    check(guestA.outcomeText() == host.outcomeText(), QStringLiteral("same outcome text"));
    check(guestA.journal().size() > 3, QStringLiteral("the journal travelled along"));

    // A guest that drops out hands its seat back to the host.
    const QVariantList lost = guestA.mySeats();
    guestA.leaveNetwork();
    check(waitFor([&] { return host.peerCount() == 1; }), QStringLiteral("the host noticed"));
    check(waitFor([&] {
        const QVariantList owners = host.seatOwners();
        return owners[lost[0].toInt()].toMap().value(QStringLiteral("mine")).toBool();
    }), QStringLiteral("the empty seat is played on the host again"));

    host.leaveNetwork();
    guestB.leaveNetwork();
    pump(50);

    std::printf(failures == 0 ? "all lan checks passed\n" : "%d lan checks failed\n", failures);
    return failures == 0 ? 0 : 1;
}
