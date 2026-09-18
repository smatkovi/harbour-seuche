/*
    Copyright (C) 2026 smatkovi

    SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "Snapshot.h"

#include <QVariantList>

namespace seuche {
namespace wire {
namespace {

QVariantList cardIds(const PlayerCards &cards)
{
    QVariantList list;
    for (PlayerCard card : cards)
        list.append(int(card.id));
    return list;
}

PlayerCards toCards(const QVariantList &list)
{
    PlayerCards cards;
    for (const QVariant &value : list)
        cards.push_back(PlayerCard(std::uint8_t(value.toInt())));
    return cards;
}

QVariantList cityIds(const InfectionCards &cities)
{
    QVariantList list;
    for (City city : cities)
        list.append(int(city.id));
    return list;
}

InfectionCards toCities(const QVariantList &list)
{
    InfectionCards cities;
    for (const QVariant &value : list)
        cities.push_back(City(std::uint8_t(value.toInt())));
    return cities;
}

} // namespace

QVariantMap toSnapshot(const Game &game)
{
    QVariantMap map;

    QVariantList cubes;
    for (int city = 0; city < kCities; ++city) {
        for (int colour = 0; colour < kColourSlots; ++colour)
            cubes.append(int(game.cubes[city][colour]));
    }
    map[QStringLiteral("cubes")] = cubes;

    QVariantList stations;
    for (int city = 0; city < kCities; ++city) {
        if (game.stations.test(city))
            stations.append(city);
    }
    map[QStringLiteral("stations")] = stations;

    QVariantList supply, cures;
    for (int colour = 0; colour < kColourSlots; ++colour) {
        supply.append(int(game.supply[colour]));
        cures.append(int(game.cures[colour]));
    }
    map[QStringLiteral("supply")] = supply;
    map[QStringLiteral("cures")] = cures;
    map[QStringLiteral("colours")] = int(game.colours);

    QVariantList players;
    for (const Player &player : game.players) {
        QVariantMap seat;
        seat[QStringLiteral("role")] = int(player.role);
        seat[QStringLiteral("city")] = int(player.city.id);
        seat[QStringLiteral("hand")] = cardIds(player.hand);
        seat[QStringLiteral("stored")] = int(player.storedEvent.id);
        seat[QStringLiteral("opsUsed")] = player.operationsFlightUsed;
        players.append(seat);
    }
    map[QStringLiteral("players")] = players;

    map[QStringLiteral("atTurn")] = int(game.atTurn);
    map[QStringLiteral("actionsLeft")] = int(game.actionsLeft);
    map[QStringLiteral("drawsLeft")] = int(game.drawsLeft);
    map[QStringLiteral("discarding")] = int(game.discardingPlayer);

    map[QStringLiteral("playerDeck")] = int(game.playerDeck.size());
    map[QStringLiteral("infectionDeck")] = int(game.infectionDeck.size());
    map[QStringLiteral("playerDiscard")] = cardIds(game.playerDiscard);
    map[QStringLiteral("infectionDiscard")] = cityIds(game.infectionDiscard);
    map[QStringLiteral("removed")] = cityIds(game.removedFromGame);

    map[QStringLiteral("outbreaks")] = int(game.outbreaks);
    map[QStringLiteral("rate")] = int(game.infectionRateIndex);
    map[QStringLiteral("infectionsLeft")] = int(game.infectionsLeft);
    map[QStringLiteral("quietNight")] = game.quietNight;
    map[QStringLiteral("difficulty")] = int(game.difficulty);
    map[QStringLiteral("phase")] = int(game.phase);
    map[QStringLiteral("outcome")] = int(game.outcome);
    return map;
}

bool fromSnapshot(const QVariantMap &snapshot, Game &game)
{
    const QVariantList players = snapshot.value(QStringLiteral("players")).toList();
    const QVariantList cubes = snapshot.value(QStringLiteral("cubes")).toList();
    if (players.isEmpty() || cubes.size() != kCities * kColourSlots)
        return false;

    game = Game();
    for (int city = 0; city < kCities; ++city) {
        for (int colour = 0; colour < kColourSlots; ++colour)
            game.cubes[city][colour] = std::uint8_t(cubes[city * kColourSlots + colour].toInt());
    }

    for (const QVariant &value : snapshot.value(QStringLiteral("stations")).toList())
        game.stations.set(value.toInt());

    const QVariantList supply = snapshot.value(QStringLiteral("supply")).toList();
    const QVariantList cures = snapshot.value(QStringLiteral("cures")).toList();
    for (int colour = 0; colour < kColourSlots && colour < supply.size(); ++colour) {
        game.supply[colour] = std::uint8_t(supply[colour].toInt());
        game.cures[colour] = CureState(cures.value(colour).toInt());
    }
    game.colours = std::uint8_t(snapshot.value(QStringLiteral("colours"), kBaseColours).toInt());

    game.players.resize(players.size());
    for (int seat = 0; seat < players.size(); ++seat) {
        const QVariantMap entry = players[seat].toMap();
        Player &player = game.players[seat];
        player.role = Role(entry.value(QStringLiteral("role")).toInt());
        player.city = City(std::uint8_t(entry.value(QStringLiteral("city")).toInt()));
        player.hand = toCards(entry.value(QStringLiteral("hand")).toList());
        player.storedEvent = PlayerCard(std::uint8_t(entry.value(QStringLiteral("stored"), 0xFF).toInt()));
        player.operationsFlightUsed = entry.value(QStringLiteral("opsUsed")).toBool();
    }

    game.atTurn = std::uint8_t(snapshot.value(QStringLiteral("atTurn")).toInt());
    game.actionsLeft = std::uint8_t(snapshot.value(QStringLiteral("actionsLeft")).toInt());
    game.drawsLeft = std::uint8_t(snapshot.value(QStringLiteral("drawsLeft")).toInt());
    game.discardingPlayer = std::uint8_t(snapshot.value(QStringLiteral("discarding"), 0xFF).toInt());

    // blanks: the right count, no order — that one stays with the host
    game.playerDeck.assign(snapshot.value(QStringLiteral("playerDeck")).toInt(), PlayerCard());
    game.infectionDeck.assign(snapshot.value(QStringLiteral("infectionDeck")).toInt(), City());
    game.playerDiscard = toCards(snapshot.value(QStringLiteral("playerDiscard")).toList());
    game.infectionDiscard = toCities(snapshot.value(QStringLiteral("infectionDiscard")).toList());
    game.removedFromGame = toCities(snapshot.value(QStringLiteral("removed")).toList());

    game.outbreaks = std::uint8_t(snapshot.value(QStringLiteral("outbreaks")).toInt());
    game.infectionRateIndex = std::uint8_t(snapshot.value(QStringLiteral("rate")).toInt());
    game.infectionsLeft = std::uint8_t(snapshot.value(QStringLiteral("infectionsLeft")).toInt());
    game.quietNight = snapshot.value(QStringLiteral("quietNight")).toBool();
    game.difficulty = Difficulty(snapshot.value(QStringLiteral("difficulty")).toInt());
    game.phase = Phase(snapshot.value(QStringLiteral("phase")).toInt());
    game.outcome = Outcome(snapshot.value(QStringLiteral("outcome")).toInt());
    return true;
}

QVariantMap toVariant(const Action &action)
{
    QVariantMap map;
    map[QStringLiteral("kind")] = int(action.kind);
    map[QStringLiteral("pawn")] = int(action.pawn);
    map[QStringLiteral("target")] = int(action.target.id);
    map[QStringLiteral("other")] = int(action.otherSeat);
    map[QStringLiteral("card")] = int(action.card.id);
    map[QStringLiteral("colour")] = int(action.colour);
    map[QStringLiteral("cure")] = cardIds(action.cure);
    return map;
}

Action actionFromVariant(const QVariantMap &map)
{
    Action action;
    action.kind = ActionKind(map.value(QStringLiteral("kind")).toInt());
    action.pawn = std::uint8_t(map.value(QStringLiteral("pawn"), 0xFF).toInt());
    action.target = City(std::uint8_t(map.value(QStringLiteral("target"), 0xFF).toInt()));
    action.otherSeat = std::uint8_t(map.value(QStringLiteral("other"), 0xFF).toInt());
    action.card = PlayerCard(std::uint8_t(map.value(QStringLiteral("card"), 0xFF).toInt()));
    action.colour = Colour(map.value(QStringLiteral("colour")).toInt());
    action.cure = toCards(map.value(QStringLiteral("cure")).toList());
    return action;
}

QVariantMap toVariant(const EventPlay &play)
{
    QVariantMap map;
    map[QStringLiteral("seat")] = int(play.seat);
    map[QStringLiteral("event")] = int(play.event);
    map[QStringLiteral("role")] = play.fromRoleCard;
    map[QStringLiteral("city")] = int(play.city.id);
    map[QStringLiteral("pawn")] = int(play.pawn);
    map[QStringLiteral("removed")] = int(play.removed.id);
    map[QStringLiteral("order")] = cityIds(play.order);
    return map;
}

EventPlay eventFromVariant(const QVariantMap &map)
{
    EventPlay play;
    play.seat = std::uint8_t(map.value(QStringLiteral("seat"), 0xFF).toInt());
    play.event = Event(map.value(QStringLiteral("event")).toInt());
    play.fromRoleCard = map.value(QStringLiteral("role")).toBool();
    play.city = City(std::uint8_t(map.value(QStringLiteral("city"), 0xFF).toInt()));
    play.pawn = std::uint8_t(map.value(QStringLiteral("pawn"), 0xFF).toInt());
    play.removed = City(std::uint8_t(map.value(QStringLiteral("removed"), 0xFF).toInt()));
    play.order = toCities(map.value(QStringLiteral("order")).toList());
    return play;
}

} // namespace wire
} // namespace seuche
