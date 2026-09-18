/*
    Copyright (C) 2026 smatkovi

    SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "SeucheEngine.h"

#include "net/Snapshot.h"

#include <QVariantMap>

#include <algorithm>
#include <chrono>

using namespace seuche;

namespace {

const char *kColourNames[] = {"Blau", "Gelb", "Schwarz", "Rot", "Violett"};

QString colourName(Colour colour)
{
    return QString::fromUtf8(kColourNames[int(colour)]);
}

// Normalised board coordinates. The map is drawn from the real positions, so the
// bounds are taken from the data instead of from a hard coded world rectangle.
void boardBounds(float &minLat, float &maxLat, float &minLon, float &maxLon)
{
    minLat = minLon = 1000.0f;
    maxLat = maxLon = -1000.0f;
    for (int id = 0; id < kCities; ++id) {
        const CityInfo &info = cityInfo(City(std::uint8_t(id)));
        minLat = std::min(minLat, info.latitude);
        maxLat = std::max(maxLat, info.latitude);
        minLon = std::min(minLon, info.longitude);
        maxLon = std::max(maxLon, info.longitude);
    }
}

} // namespace

SeucheEngine::SeucheEngine(QObject *parent)
    : QObject(parent)
    , rng_(std::mt19937(std::random_device{}()))
{
    connect(&session_, &LanSession::messageReceived, this, &SeucheEngine::handleMessage);
    connect(&session_, &LanSession::peerJoined, this, &SeucheEngine::assignSeat);
    connect(&session_, &LanSession::peerLost, this, &SeucheEngine::releaseSeats);
    connect(&session_, &LanSession::peerConnectedChanged, this, &SeucheEngine::netChanged);
    connect(&session_, &LanSession::connectionFailed, this, [this](const QString &reason) {
        setStatus(tr("Verbindung fehlgeschlagen: %1").arg(reason));
    });
}

QString SeucheEngine::phase() const
{
    if (!running_)
        return QStringLiteral("none");
    switch (game_.phase) {
    case Phase::Actions: return QStringLiteral("actions");
    case Phase::Draw:    return QStringLiteral("draw");
    case Phase::Discard: return QStringLiteral("discard");
    case Phase::Infect:  return QStringLiteral("infect");
    case Phase::Over:    return QStringLiteral("over");
    }
    return QStringLiteral("none");
}

QString SeucheEngine::phaseText() const
{
    if (!running_)
        return QString();
    if (game_.over())
        return outcomeText();

    switch (game_.phase) {
    case Phase::Actions:
        return tr("%1: %2 Aktionen").arg(seatName(game_.atTurn)).arg(game_.actionsLeft);
    case Phase::Draw:
        return tr("%1 zieht %2 Karten").arg(seatName(game_.atTurn)).arg(game_.drawsLeft);
    case Phase::Discard:
        return tr("%1 muss abwerfen").arg(seatName(game_.discardingPlayer));
    case Phase::Infect:
        return tr("Infektion: noch %1").arg(game_.infectionsLeft);
    case Phase::Over:
        return outcomeText();
    }
    return QString();
}

QString SeucheEngine::outcomeText() const
{
    if (!running_)
        return QString();
    switch (game_.outcome) {
    case Outcome::Running:       return QString();
    case Outcome::Won:           return tr("Gewonnen: alle vier Heilmittel gefunden");
    case Outcome::LostOutbreaks: return tr("Verloren: acht Ausbrüche");
    case Outcome::LostCubes:     return tr("Verloren: keine Würfel mehr im Vorrat");
    case Outcome::LostCards:     return tr("Verloren: das Spielerdeck ist leer");
    }
    return QString();
}

int SeucheEngine::discardingSeat() const
{
    if (!running_ || game_.discardingPlayer == 0xFF)
        return -1;
    return game_.discardingPlayer;
}

QString SeucheEngine::seatName(int seat) const
{
    if (!running_ || seat < 0 || seat >= int(game_.players.size()))
        return QString();
    return QString::fromUtf8(roleName(game_.players[seat].role));
}

QString SeucheEngine::cityName(int cityId) const
{
    const City city{std::uint8_t(cityId)};
    return city.valid() ? QString::fromUtf8(cityInfo(city).name) : QString();
}

QString SeucheEngine::cardLabel(int cardId) const
{
    const PlayerCard card{std::uint8_t(cardId)};
    if (card.isCity())
        return cityName(card.city().id);
    if (card.isEvent())
        return QString::fromUtf8(eventName(card.event()));
    if (card.isEpidemic())
        return tr("Epidemie");
    return QString();
}

QVariantList SeucheEngine::cities() const
{
    QVariantList list;
    if (!running_)
        return list;

    float minLat, maxLat, minLon, maxLon;
    boardBounds(minLat, maxLat, minLon, maxLon);
    const float spanLat = maxLat - minLat;
    const float spanLon = maxLon - minLon;

    for (int id = 0; id < kCities; ++id) {
        const City city{std::uint8_t(id)};
        const CityInfo &info = cityInfo(city);

        QVariantList cubes;
        int total = 0;
        for (int colour = 0; colour < game_.colourCount(); ++colour) {
            const int count = game_.cubes[id][colour];
            cubes.append(count);
            total += count;
        }
        QVariantList pawns;
        for (int seat = 0; seat < int(game_.players.size()); ++seat) {
            if (game_.players[seat].city == city)
                pawns.append(seat);
        }

        QVariantMap entry;
        entry[QStringLiteral("id")] = id;
        entry[QStringLiteral("name")] = QString::fromUtf8(info.name);
        entry[QStringLiteral("colour")] = int(city.colour());
        entry[QStringLiteral("x")] = (info.longitude - minLon) / spanLon;
        entry[QStringLiteral("y")] = (maxLat - info.latitude) / spanLat;
        entry[QStringLiteral("cubes")] = cubes;
        entry[QStringLiteral("cubeCount")] = total;
        entry[QStringLiteral("station")] = game_.hasStation(city);
        entry[QStringLiteral("pawns")] = pawns;
        list.append(entry);
    }
    return list;
}

QVariantList SeucheEngine::links() const
{
    float minLat, maxLat, minLon, maxLon;
    boardBounds(minLat, maxLat, minLon, maxLon);
    const float spanLat = maxLat - minLat;
    const float spanLon = maxLon - minLon;

    QVariantList list;
    for (int id = 0; id < kCities; ++id) {
        const City city{std::uint8_t(id)};
        const CityInfo &from = cityInfo(city);
        for (City other : neighbours(city)) {
            if (other.id < id)
                continue;                       // every edge once
            const CityInfo &to = cityInfo(other);
            QVariantMap entry;
            entry[QStringLiteral("x1")] = (from.longitude - minLon) / spanLon;
            entry[QStringLiteral("y1")] = (maxLat - from.latitude) / spanLat;
            entry[QStringLiteral("x2")] = (to.longitude - minLon) / spanLon;
            entry[QStringLiteral("y2")] = (maxLat - to.latitude) / spanLat;
            list.append(entry);
        }
    }
    return list;
}

QVariantList SeucheEngine::players() const
{
    QVariantList list;
    if (!running_)
        return list;

    for (int seat = 0; seat < int(game_.players.size()); ++seat) {
        const Player &player = game_.players[seat];
        QVariantList hand;
        for (PlayerCard card : player.hand) {
            QVariantMap entry;
            entry[QStringLiteral("id")] = int(card.id);
            entry[QStringLiteral("label")] = cardLabel(card.id);
            entry[QStringLiteral("colour")] = card.isCity() ? int(card.colour()) : -1;
            entry[QStringLiteral("event")] = card.isEvent();
            hand.append(entry);
        }

        QVariantMap entry;
        entry[QStringLiteral("seat")] = seat;
        entry[QStringLiteral("role")] = QString::fromUtf8(roleName(player.role));
        entry[QStringLiteral("ability")] = QString::fromUtf8(roleAbility(player.role));
        entry[QStringLiteral("city")] = int(player.city.id);
        entry[QStringLiteral("cityName")] = cityName(player.city.id);
        entry[QStringLiteral("hand")] = hand;
        entry[QStringLiteral("stored")] = player.storedEvent.valid()
            ? cardLabel(player.storedEvent.id) : QString();
        entry[QStringLiteral("atTurn")] = seat == game_.atTurn;
        list.append(entry);
    }
    return list;
}

QVariantList SeucheEngine::cures() const
{
    QVariantList list;
    if (!running_)
        return list;
    for (int colour = 0; colour < game_.colourCount(); ++colour) {
        QVariantMap entry;
        entry[QStringLiteral("colour")] = colour;
        entry[QStringLiteral("name")] = colourName(Colour(colour));
        entry[QStringLiteral("cured")] = game_.cures[colour] != CureState::None;
        entry[QStringLiteral("eradicated")] = game_.cures[colour] == CureState::Eradicated;
        entry[QStringLiteral("supply")] = int(game_.supply[colour]);
        list.append(entry);
    }
    return list;
}

void SeucheEngine::newGame(int seats, int difficulty)
{
    if (isGuest())
        return;                       // only the host deals

    SetupOptions options;
    options.players = std::max(2, std::min(4, seats));
    options.difficulty = Difficulty(std::max(0, std::min(2, difficulty)));

    rng_.seed(std::mt19937::result_type(
        std::chrono::system_clock::now().time_since_epoch().count()));
    game_ = seuche::newGame(options, rng_);
    running_ = true;
    journal_.clear();
    note(tr("Neue Partie: %1 Personen").arg(options.players));
    if (isHost()) {
        seatOwner_.assign(options.players, -1);
        sendState();
    }
    refresh();
}

QString SeucheEngine::label(const Action &action) const
{
    const Player &player = game_.current();
    const int seat = action.pawn == 0xFF ? game_.atTurn : action.pawn;
    const QString who = seat == game_.atTurn ? QString()
                                             : tr(" (%1)").arg(seatName(seat));

    switch (action.kind) {
    case ActionKind::Drive:
        return tr("Fahrt nach %1").arg(cityName(action.target.id)) + who;
    case ActionKind::DirectFlight:
        return tr("Direktflug nach %1").arg(cityName(action.target.id)) + who;
    case ActionKind::CharterFlight:
        return tr("Charterflug nach %1 (Karte %2)")
            .arg(cityName(action.target.id))
            .arg(cityName(game_.players[seat].city.id)) + who;
    case ActionKind::ShuttleFlight:
        return tr("Shuttle nach %1").arg(cityName(action.target.id)) + who;
    case ActionKind::BuildStation:
        return action.target.valid()
            ? tr("Station bauen (versetzt %1)").arg(cityName(action.target.id))
            : tr("Forschungsstation bauen");
    case ActionKind::Treat:
        return tr("%1 behandeln").arg(colourName(action.colour));
    case ActionKind::ShareGive:
        return tr("%1 an %2 geben").arg(cardLabel(action.card.id))
                                   .arg(seatName(action.otherSeat));
    case ActionKind::ShareTake:
        return tr("%1 von %2 nehmen").arg(cardLabel(action.card.id))
                                     .arg(seatName(action.otherSeat));
    case ActionKind::DiscoverCure:
        return tr("Heilmittel %1 entdecken").arg(colourName(action.colour));
    case ActionKind::OperationsFlight:
        return tr("Flug nach %1 (Karte %2)").arg(cityName(action.target.id))
                                            .arg(cardLabel(action.card.id));
    case ActionKind::PlannerTake:
        return tr("%1 aufbewahren").arg(cardLabel(action.card.id));
    case ActionKind::Pass:
        return player.hand.empty() ? tr("Zug beenden") : tr("Zug beenden");
    }
    return QString();
}

void SeucheEngine::refresh()
{
    legal_ = running_ ? legalActions(game_) : std::vector<Action>();
    emit changed();
}

void SeucheEngine::note(const QString &line)
{
    journal_.append(line);
    while (journal_.size() > 200)
        journal_.removeFirst();
}

QVariantList SeucheEngine::allActions() const
{
    QVariantList list;
    for (std::size_t index = 0; index < legal_.size(); ++index) {
        const Action &action = legal_[index];
        const int seat = action.pawn == 0xFF ? game_.atTurn : action.pawn;

        int city = -1;
        switch (action.kind) {
        case ActionKind::Drive:
        case ActionKind::DirectFlight:
        case ActionKind::CharterFlight:
        case ActionKind::ShuttleFlight:
        case ActionKind::OperationsFlight:
            city = action.target.id;
            break;
        case ActionKind::Pass:
            break;
        default:
            city = game_.current().city.id;     // happens where the pawn stands
            break;
        }

        QVariantMap entry;
        entry[QStringLiteral("index")] = int(index);
        entry[QStringLiteral("label")] = label(action);
        entry[QStringLiteral("city")] = city;
        entry[QStringLiteral("kind")] = int(action.kind);
        entry[QStringLiteral("seat")] = seat;
        list.append(entry);
    }
    return list;
}

QVariantList SeucheEngine::actionsForCity(int cityId) const
{
    QVariantList list;
    for (const QVariant &entry : allActions()) {
        if (entry.toMap().value(QStringLiteral("city")).toInt() == cityId)
            list.append(entry);
    }
    return list;
}

bool SeucheEngine::run(int index)
{
    if (!running_ || index < 0 || index >= int(legal_.size()) || !mayAct())
        return false;

    const Action action = legal_[index];
    if (isGuest()) {
        QVariantMap message;
        message[QStringLiteral("t")] = QStringLiteral("action");
        message[QStringLiteral("a")] = wire::toVariant(action);
        session_.send(message);
        return true;              // the host answers with the new state
    }
    return applyAction(action);
}

bool SeucheEngine::applyAction(const Action &action)
{
    const QString text = label(action);
    const int outbreaksBefore = game_.outbreaks;

    if (apply(game_, action, rng_) != Reason::Ok)
        return false;

    note(text);
    if (game_.outbreaks > outbreaksBefore)
        note(tr("Ausbrüche: %1").arg(game_.outbreaks));
    if (game_.over())
        note(outcomeText());
    refresh();
    sendState();
    return true;
}

void SeucheEngine::drawCard()
{
    if (!running_ || game_.phase != Phase::Draw || !mayAct())
        return;
    if (isGuest()) {
        QVariantMap message;
        message[QStringLiteral("t")] = QStringLiteral("step");
        message[QStringLiteral("s")] = QStringLiteral("draw");
        session_.send(message);
        return;
    }
    applyDraw();
}

void SeucheEngine::applyDraw()
{

    const int before = int(game_.playerDeck.size());
    const PlayerCard top = before > 0 ? game_.playerDeck.back() : PlayerCard();
    const int rateBefore = game_.infectionRateIndex;

    drawPlayerCard(game_, rng_);

    if (before == 0) {
        note(outcomeText());
    } else if (top.isEpidemic()) {
        note(tr("Epidemie! Infektionsrate %1").arg(game_.infectionRate()));
        if (game_.infectionRateIndex == rateBefore)
            note(tr("Die Rate steht schon am Ende"));
        if (!game_.infectionDiscard.empty() || !game_.infectionDeck.empty())
            note(tr("Ablagestapel gemischt und obenauf gelegt"));
    } else {
        note(tr("Gezogen: %1").arg(cardLabel(top.id)));
    }
    if (game_.over())
        note(outcomeText());
    refresh();
    sendState();
}

void SeucheEngine::infectCity()
{
    if (!running_ || game_.phase != Phase::Infect || !mayAct())
        return;
    if (isGuest()) {
        QVariantMap message;
        message[QStringLiteral("t")] = QStringLiteral("step");
        message[QStringLiteral("s")] = QStringLiteral("infect");
        session_.send(message);
        return;
    }
    applyInfect();
}

void SeucheEngine::applyInfect()
{

    const City next = game_.infectionDeck.empty() ? City() : game_.infectionDeck.back();
    const int outbreaksBefore = game_.outbreaks;

    infectStep(game_);

    if (next.valid()) {
        note(tr("Infektion: %1").arg(cityName(next.id)));
        if (game_.outbreaks > outbreaksBefore)
            note(tr("Ausbruch! Ausbrüche: %1").arg(game_.outbreaks));
    }
    if (game_.over())
        note(outcomeText());
    refresh();
    sendState();
}

bool SeucheEngine::discardCard(int cardId)
{
    if (!running_ || game_.phase != Phase::Discard || !mayAct())
        return false;
    if (isGuest()) {
        QVariantMap message;
        message[QStringLiteral("t")] = QStringLiteral("discard");
        message[QStringLiteral("card")] = cardId;
        session_.send(message);
        return true;
    }
    return applyDiscardCard(game_.discardingPlayer, cardId);
}

bool SeucheEngine::applyDiscardCard(int seat, int cardId)
{
    if (!running_ || game_.phase != Phase::Discard || seat != game_.discardingPlayer)
        return false;
    if (discard(game_, seat, PlayerCard(std::uint8_t(cardId))) != Reason::Ok)
        return false;
    note(tr("Abgeworfen: %1").arg(cardLabel(cardId)));
    refresh();
    sendState();
    return true;
}

QVariantList SeucheEngine::eventCards() const
{
    QVariantList list;
    if (!running_ || game_.over())
        return list;

    for (int seat = 0; seat < int(game_.players.size()); ++seat) {
        const Player &player = game_.players[seat];
        PlayerCards holdings = player.hand;
        if (player.storedEvent.valid())
            holdings.push_back(player.storedEvent);

        for (PlayerCard card : holdings) {
            if (!card.isEvent())
                continue;
            const Event event = card.event();

            QString needs;
            switch (event) {
            case Event::QuietNight: needs = QString(); break;
            case Event::Forecast:   needs = QStringLiteral("forecast"); break;
            case Event::Grant:      needs = QStringLiteral("city"); break;
            case Event::Airlift:    needs = QStringLiteral("pawn"); break;
            case Event::Resilient:  needs = QStringLiteral("infection"); break;
            }

            EventPlay probe;
            probe.seat = std::uint8_t(seat);
            probe.event = event;
            probe.fromRoleCard = !hasCard(player.hand, card);
            if (needs.isEmpty() && check(game_, probe) != Reason::Ok)
                continue;

            QVariantMap entry;
            entry[QStringLiteral("seat")] = seat;
            entry[QStringLiteral("event")] = int(event);
            entry[QStringLiteral("name")] = QString::fromUtf8(eventName(event));
            entry[QStringLiteral("text")] = QString::fromUtf8(eventText(event));
            entry[QStringLiteral("holder")] = seatName(seat);
            entry[QStringLiteral("needs")] = needs;
            list.append(entry);
        }
    }
    return list;
}

bool SeucheEngine::playEvent(int seat, int event, int cityId, int pawn)
{
    if (!running_ || seat < 0 || seat >= int(game_.players.size()) || !ownsSeat(seat))
        return false;

    EventPlay play;
    play.seat = std::uint8_t(seat);
    play.event = Event(event);
    play.city = City(std::uint8_t(cityId));
    play.pawn = std::uint8_t(pawn);
    play.removed = City(std::uint8_t(cityId));
    play.fromRoleCard = !hasCard(game_.players[seat].hand, eventCard(Event(event)));
    return applyEventPlay(play);
}

bool SeucheEngine::applyEventPlay(const EventPlay &play)
{
    if (isGuest()) {
        QVariantMap message;
        message[QStringLiteral("t")] = QStringLiteral("event");
        message[QStringLiteral("e")] = wire::toVariant(play);
        session_.send(message);
        return true;
    }
    if (apply(game_, play, rng_) != Reason::Ok)
        return false;
    note(tr("%1: %2").arg(seatName(play.seat))
                     .arg(QString::fromUtf8(eventName(play.event))));
    refresh();
    sendState();
    return true;
}

QVariantList SeucheEngine::infectionDiscard() const
{
    QVariantList list;
    if (!running_)
        return list;
    for (City city : game_.infectionDiscard) {
        QVariantMap entry;
        entry[QStringLiteral("id")] = int(city.id);
        entry[QStringLiteral("name")] = cityName(city.id);
        entry[QStringLiteral("colour")] = int(city.colour());
        list.append(entry);
    }
    return list;
}

QVariantList SeucheEngine::forecastCards() const
{
    QVariantList list;
    if (!running_ || int(game_.infectionDeck.size()) < kForecastCards)
        return list;
    for (auto it = game_.infectionDeck.end() - kForecastCards; it != game_.infectionDeck.end(); ++it) {
        QVariantMap entry;
        entry[QStringLiteral("id")] = int(it->id);
        entry[QStringLiteral("name")] = cityName(it->id);
        entry[QStringLiteral("colour")] = int(it->colour());
        list.append(entry);
    }
    return list;
}

bool SeucheEngine::playForecast(int seat, const QVariantList &order)
{
    if (!running_ || seat < 0 || seat >= int(game_.players.size()) || !ownsSeat(seat))
        return false;

    EventPlay play;
    play.seat = std::uint8_t(seat);
    play.event = Event::Forecast;
    play.fromRoleCard = !hasCard(game_.players[seat].hand, eventCard(Event::Forecast));
    // The page lists them in drawing order, the deck has its top at the back.
    for (auto it = order.crbegin(); it != order.crend(); ++it)
        play.order.push_back(City(std::uint8_t(it->toInt())));

    return applyEventPlay(play);
}


// --- network ---------------------------------------------------------------

QString SeucheEngine::netRole() const
{
    switch (session_.role()) {
    case LanSession::Host:  return QStringLiteral("host");
    case LanSession::Guest: return QStringLiteral("guest");
    case LanSession::None:  break;
    }
    return QStringLiteral("local");
}

QVariantList SeucheEngine::mySeats() const
{
    QVariantList list;
    if (!running_)
        return list;
    for (int seat = 0; seat < int(game_.players.size()); ++seat) {
        if (ownsSeat(seat))
            list.append(seat);
    }
    return list;
}

int SeucheEngine::ownerOfSeat(int seat) const
{
    if (seat < 0 || seat >= int(seatOwner_.size()))
        return -1;
    return seatOwner_[seat];
}

bool SeucheEngine::ownsSeat(int seat) const
{
    if (session_.role() == LanSession::None)
        return true;                              // hot seat: everything is ours
    if (isHost())
        return ownerOfSeat(seat) < 0;             // whatever was not given away
    return std::find(mySeats_.begin(), mySeats_.end(), seat) != mySeats_.end();
}

int SeucheEngine::seatToAct() const
{
    if (!running_)
        return -1;
    if (game_.phase == Phase::Discard)
        return game_.discardingPlayer;
    return game_.atTurn;
}

bool SeucheEngine::mayAct() const
{
    if (!running_ || game_.over())
        return false;
    return ownsSeat(seatToAct());
}

QVariantList SeucheEngine::seatOwners() const
{
    QVariantList list;
    if (!running_)
        return list;
    for (int seat = 0; seat < int(game_.players.size()); ++seat) {
        QVariantMap entry;
        entry[QStringLiteral("seat")] = seat;
        entry[QStringLiteral("role")] = QString::fromUtf8(roleName(game_.players[seat].role));
        entry[QStringLiteral("mine")] = ownsSeat(seat);
        entry[QStringLiteral("peer")] = ownerOfSeat(seat);
        list.append(entry);
    }
    return list;
}

void SeucheEngine::setStatus(const QString &text)
{
    netStatus_ = text;
    emit netChanged();
}

bool SeucheEngine::hostGame(int seats, int difficulty, const QString &name)
{
    session_.stop();
    seatOwner_.clear();
    mySeats_.clear();

    QString error;
    const QString shown = name.isEmpty() ? tr("Seuche") : name;
    if (!session_.startHosting(shown, seats, seats - 1, &error)) {
        setStatus(tr("Konnte nicht öffnen: %1").arg(error));
        return false;
    }
    newGame(seats, difficulty);                   // deals and sends the state
    seatOwner_.assign(seats, -1);
    setStatus(tr("Offen für %1 Geräte").arg(seats - 1));
    emit netChanged();
    return true;
}

void SeucheEngine::joinGame(const QString &address)
{
    session_.stop();
    running_ = false;
    seatOwner_.clear();
    mySeats_.clear();
    session_.joinHost(LanSession::normalizeAddress(address));
    setStatus(tr("Verbinde mit %1 …").arg(address));
    emit changed();
}

void SeucheEngine::leaveNetwork()
{
    session_.stop();
    seatOwner_.clear();
    mySeats_.clear();
    setStatus(QString());
    emit changed();
}

void SeucheEngine::sendState()
{
    if (!isHost() || !running_)
        return;

    QVariantList owners;
    for (int owner : seatOwner_)
        owners.append(owner);

    QVariantMap message;
    message[QStringLiteral("t")] = QStringLiteral("state");
    message[QStringLiteral("g")] = wire::toSnapshot(game_);
    message[QStringLiteral("journal")] = QStringList(journal_.mid(qMax(0, journal_.size() - 40)));
    message[QStringLiteral("owners")] = owners;
    session_.send(message);
}

void SeucheEngine::assignSeat(int peer)
{
    if (!isHost() || !running_)
        return;

    // Seat 0 stays with the host; every guest gets the next free one.
    int given = -1;
    for (int seat = 1; seat < int(seatOwner_.size()); ++seat) {
        if (seatOwner_[seat] < 0) {
            seatOwner_[seat] = peer;
            given = seat;
            break;
        }
    }

    QVariantMap welcome;
    welcome[QStringLiteral("t")] = QStringLiteral("welcome");
    welcome[QStringLiteral("seat")] = given;
    session_.sendTo(peer, welcome);
    sendState();

    note(given >= 0 ? tr("Gerät übernimmt %1").arg(seatName(given))
                    : tr("Ein Gerät sieht zu"));
    setStatus(tr("%1 Geräte verbunden").arg(session_.peerCount()));
    refresh();
}

void SeucheEngine::releaseSeats(int peer)
{
    for (int seat = 0; seat < int(seatOwner_.size()); ++seat) {
        if (seatOwner_[seat] == peer) {
            seatOwner_[seat] = -1;
            note(tr("%1 wird wieder hier gespielt").arg(seatName(seat)));
        }
    }
    setStatus(tr("%1 Geräte verbunden").arg(session_.peerCount()));
    sendState();
    refresh();
}

void SeucheEngine::handleMessage(int peer, const QVariantMap &message)
{
    const QString type = message.value(QStringLiteral("t")).toString();

    if (isGuest()) {
        if (type == QLatin1String("state")) {
            if (!wire::fromSnapshot(message.value(QStringLiteral("g")).toMap(), game_))
                return;
            running_ = true;
            journal_ = message.value(QStringLiteral("journal")).toStringList();
            const QVariantList owners = message.value(QStringLiteral("owners")).toList();
            seatOwner_.clear();
            for (const QVariant &owner : owners)
                seatOwner_.push_back(owner.toInt());
            refresh();
            emit netChanged();
        } else if (type == QLatin1String("welcome")) {
            mySeats_.clear();
            const int seat = message.value(QStringLiteral("seat")).toInt();
            if (seat >= 0) {
                mySeats_.push_back(seat);
                setStatus(tr("Verbunden — Sitz %1").arg(seat + 1));
            } else {
                setStatus(tr("Verbunden — Zuschauer"));
            }
        }
        return;
    }

    if (!isHost() || !running_)
        return;

    // The host is the referee: a device may only move the seat it holds.
    if (type == QLatin1String("action")) {
        if (ownerOfSeat(seatToAct()) != peer)
            return;
        applyAction(wire::actionFromVariant(message.value(QStringLiteral("a")).toMap()));
    } else if (type == QLatin1String("step")) {
        if (ownerOfSeat(seatToAct()) != peer)
            return;
        if (message.value(QStringLiteral("s")).toString() == QLatin1String("draw"))
            applyDraw();
        else
            applyInfect();
    } else if (type == QLatin1String("discard")) {
        if (ownerOfSeat(game_.discardingPlayer) != peer)
            return;
        applyDiscardCard(game_.discardingPlayer, message.value(QStringLiteral("card")).toInt());
    } else if (type == QLatin1String("event")) {
        const EventPlay play = wire::eventFromVariant(message.value(QStringLiteral("e")).toMap());
        if (ownerOfSeat(play.seat) != peer)
            return;
        applyEventPlay(play);
    }
}
