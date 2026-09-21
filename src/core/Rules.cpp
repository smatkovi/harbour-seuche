#include "Rules.h"

#include <algorithm>

namespace seuche {
namespace {

// The seat whose pawn an action moves: the player at turn, unless the Dispatcher
// names someone else. Cards always come from the hand of the player at turn.
int movingSeat(const Game& game, const Action& action)
{
    return action.pawn == 0xFF ? game.atTurn : action.pawn;
}

bool knownSeat(const Game& game, int seat)
{
    return seat >= 0 && seat < static_cast<int>(game.players.size());
}

// The Medic keeps cured colours out of the city she stands in, and sweeps them
// away when she arrives.
bool medicBlocks(const Game& game, City city, Colour colour)
{
    if (game.cures[static_cast<int>(colour)] == CureState::None)
        return false;
    const int seat = game.seatOf(Role::Medic);
    return seat != 0xFF && game.players[seat].city == city;
}

void medicSweep(Game& game)
{
    const int seat = game.seatOf(Role::Medic);
    if (seat == 0xFF)
        return;
    const City city = game.players[seat].city;
    for (int colour = 0; colour < game.colourCount(); ++colour) {
        if (game.cures[colour] != CureState::None)
            removeCubes(game, city, Colour(colour), kCubesPerCity);
    }
}

void addCube(Game& game, City city, Colour colour, CitySet& outbroken)
{
    if (game.over() || !city.valid())
        return;
    const int index = static_cast<int>(colour);
    if (game.cures[index] == CureState::Eradicated)
        return;
    if (game.shielded(city) || medicBlocks(game, city, colour))
        return;

    if (game.cubes[city.id][index] < kCubesPerCity) {
        if (game.supply[index] == 0) {
            game.outcome = Outcome::LostCubes;
            game.phase = Phase::Over;
            return;
        }
        ++game.cubes[city.id][index];
        --game.supply[index];
        return;
    }

    // three cubes already: the fourth is an outbreak, once per chain
    if (outbroken.test(city.id))
        return;
    outbroken.set(city.id);
    ++game.outbreaks;
    if (game.outbreaks >= kLosingOutbreaks) {
        game.outcome = Outcome::LostOutbreaks;
        game.phase = Phase::Over;
        return;
    }
    for (City next : neighbours(city)) {
        addCube(game, next, colour, outbroken);
        if (game.over())
            return;
    }
}

void checkVictory(Game& game)
{
    if (game.over())
        return;
    for (int colour = 0; colour < game.colourCount(); ++colour) {
        if (game.cures[colour] == CureState::None)
            return;
    }
    game.outcome = Outcome::Won;
    game.phase = Phase::Over;
}

// Moves into the infect phase, or straight past it after a quiet night.
void beginInfecting(Game& game)
{
    if (game.over())
        return;
    game.phase = Phase::Infect;
    if (game.quietNight) {
        game.quietNight = false;
        game.infectionsLeft = 0;
        endTurn(game);
        return;
    }
    game.infectionsLeft = static_cast<std::uint8_t>(game.infectionRate());
}

// Called after anything that can change a hand. Whoever sits over the limit has
// to discard first — that includes the receiver of a shared card in the middle of
// someone else's action phase. Where play resumes follows from the counters.
void continueTurn(Game& game)
{
    if (game.over())
        return;
    for (std::size_t seat = 0; seat < game.players.size(); ++seat) {
        if (game.players[seat].hand.size() > kHandLimit) {
            game.phase = Phase::Discard;
            game.discardingPlayer = static_cast<std::uint8_t>(seat);
            return;
        }
    }
    game.discardingPlayer = 0xFF;
    if (game.actionsLeft > 0) {
        game.phase = Phase::Actions;
        return;
    }
    if (game.drawsLeft > 0) {
        game.phase = Phase::Draw;
        return;
    }
    beginInfecting(game);
}

void resolveEpidemic(Game& game, std::mt19937& rng)
{
    // 1. increase
    if (game.infectionRateIndex + 1 < static_cast<int>(kInfectionRates.size()))
        ++game.infectionRateIndex;

    // 2. infect the bottom card with three cubes
    if (!game.infectionDeck.empty()) {
        const City city = game.infectionDeck.front();
        game.infectionDeck.erase(game.infectionDeck.begin());
        CitySet outbroken;
        for (int cube = 0; cube < kCubesPerCity && !game.over(); ++cube)
            addCube(game, city, city.colour(), outbroken);
        game.infectionDiscard.push_back(city);
    }
    if (game.over())
        return;

    // 3. intensify
    std::shuffle(game.infectionDiscard.begin(), game.infectionDiscard.end(), rng);
    game.infectionDeck.insert(game.infectionDeck.end(),
                              game.infectionDiscard.begin(), game.infectionDiscard.end());
    game.infectionDiscard.clear();
}

// Every station placement goes through here: `moved` names the station to pick up
// when all six are already on the board.
Reason placeStation(Game& game, City city, City moved)
{
    if (game.hasStation(city))
        return Reason::StationAlreadyHere;
    if (game.stationCount() >= kStations) {
        if (!moved.valid() || !game.hasStation(moved))
            return Reason::NoStationLeft;
        game.stations.reset(moved.id);
    }
    game.stations.set(city.id);
    return Reason::Ok;
}

} // namespace

// --- board -----------------------------------------------------------------

void placeCube(Game& game, City city, Colour colour)
{
    CitySet outbroken;
    addCube(game, city, colour, outbroken);
}

void removeCubes(Game& game, City city, Colour colour, int count)
{
    if (!city.valid() || count <= 0)
        return;
    const int index = static_cast<int>(colour);
    const int taken = std::min(count, static_cast<int>(game.cubes[city.id][index]));
    game.cubes[city.id][index] -= static_cast<std::uint8_t>(taken);
    game.supply[index] += static_cast<std::uint8_t>(taken);
    if (taken > 0)
        checkEradication(game, colour);
}

void checkEradication(Game& game, Colour colour)
{
    const int index = static_cast<int>(colour);
    if (game.cures[index] == CureState::Cured && game.totalCubes(colour) == 0)
        game.cures[index] = CureState::Eradicated;
}

// --- actions ---------------------------------------------------------------

Reason check(const Game& game, const Action& action)
{
    if (game.over())
        return Reason::GameOver;
    if (game.phase == Phase::Discard)
        return Reason::HandLimitFirst;
    if (game.phase != Phase::Actions)
        return Reason::WrongPhase;
    if (game.actionsLeft == 0 && action.kind != ActionKind::Pass)
        return Reason::NoActionsLeft;

    const Player& player = game.current();
    const int seat = movingSeat(game, action);
    if (!knownSeat(game, seat))
        return Reason::NoSuchSeat;
    if (seat != game.atTurn && player.role != Role::Dispatcher)
        return Reason::NotYourPawn;

    const City from = game.players[seat].city;
    const City to = action.target;

    switch (action.kind) {
    case ActionKind::Drive:
        if (!to.valid())
            return Reason::UnknownCity;
        if (to == from)
            return Reason::SameCity;
        // the Dispatcher may also send a pawn to any city holding another pawn
        if (!adjacent(from, to)) {
            if (player.role != Role::Dispatcher)
                return Reason::NotAdjacent;
            const bool pawnThere = std::any_of(game.players.begin(), game.players.end(),
                                               [to](const Player& other) { return other.city == to; });
            if (!pawnThere)
                return Reason::NotAdjacent;
        }
        return Reason::Ok;

    case ActionKind::DirectFlight:
        if (!to.valid())
            return Reason::UnknownCity;
        if (to == from)
            return Reason::SameCity;
        if (!hasCard(player.hand, cityCard(to)))
            return Reason::CardNotInHand;
        return Reason::Ok;

    case ActionKind::CharterFlight:
        if (!to.valid())
            return Reason::UnknownCity;
        if (to == from)
            return Reason::SameCity;
        if (!hasCard(player.hand, cityCard(from)))
            return Reason::CardNotInHand;
        return Reason::Ok;

    case ActionKind::ShuttleFlight:
        if (!to.valid())
            return Reason::UnknownCity;
        if (to == from)
            return Reason::SameCity;
        if (!game.hasStation(from))
            return Reason::NoStationHere;
        if (!game.hasStation(to))
            return Reason::NoStationThere;
        return Reason::Ok;

    case ActionKind::BuildStation: {
        const City here = player.city;
        if (game.hasStation(here))
            return Reason::StationAlreadyHere;
        if (player.role != Role::Operations && !hasCard(player.hand, cityCard(here)))
            return Reason::CardNotInHand;
        if (game.stationCount() >= kStations && !(to.valid() && game.hasStation(to)))
            return Reason::NoStationLeft;
        return Reason::Ok;
    }

    case ActionKind::Treat:
        if (static_cast<int>(action.colour) >= game.colourCount())
            return Reason::UnknownCity;
        if (game.cubes_(player.city, action.colour) == 0)
            return Reason::NothingToTreat;
        return Reason::Ok;

    case ActionKind::ShareGive:
    case ActionKind::ShareTake: {
        const int other = action.otherSeat;
        if (!knownSeat(game, other) || other == game.atTurn)
            return Reason::NoSuchSeat;
        if (game.players[other].city != player.city)
            return Reason::NotInSameCity;
        if (!action.card.isCity())
            return Reason::WrongShareCard;

        const bool giving = action.kind == ActionKind::ShareGive;
        const Player& giver = giving ? player : game.players[other];
        if (!hasCard(giver.hand, action.card))
            return Reason::CardNotInHand;
        if (giver.role != Role::Researcher && action.card.city() != player.city)
            return Reason::WrongShareCard;
        return Reason::Ok;
    }

    case ActionKind::DiscoverCure: {
        if (!game.hasStation(player.city))
            return Reason::NoStationHere;
        const int index = static_cast<int>(action.colour);
        if (index >= game.colourCount())
            return Reason::UnknownCity;
        if (game.cures[index] != CureState::None)
            return Reason::AlreadyCured;
        if (static_cast<int>(action.cure.size()) != cardsForCure(player.role))
            return Reason::NotEnoughCards;

        PlayerCards left = player.hand;
        for (PlayerCard card : action.cure) {
            if (!card.isCity())
                return Reason::WrongCardColour;
            if (card.colour() != action.colour)
                return Reason::WrongCardColour;
            if (!removeCard(left, card))
                return Reason::CardNotInHand;
        }
        return Reason::Ok;
    }

    case ActionKind::OperationsFlight:
        if (player.role != Role::Operations)
            return Reason::NotYourPawn;
        if (player.operationsFlightUsed)
            return Reason::NoActionsLeft;
        if (!game.hasStation(player.city))
            return Reason::NoStationHere;
        if (!to.valid())
            return Reason::UnknownCity;
        if (to == player.city)
            return Reason::SameCity;
        if (!action.card.isCity() || !hasCard(player.hand, action.card))
            return Reason::CardNotInHand;
        return Reason::Ok;

    case ActionKind::PlannerTake:
        if (player.role != Role::Planner)
            return Reason::NotYourPawn;
        if (player.storedEvent.valid())
            return Reason::AlreadyStoringEvent;
        if (!action.card.isEvent())
            return Reason::NotAnEventCard;
        if (!hasCard(game.playerDiscard, action.card))
            return Reason::NoEventInDiscard;
        return Reason::Ok;

    case ActionKind::Pass:
        return Reason::Ok;
    }
    return Reason::WrongPhase;
}

Reason apply(Game& game, const Action& action, std::mt19937& rng)
{
    (void)rng;
    const Reason reason = check(game, action);
    if (reason != Reason::Ok)
        return reason;

    Player& player = game.current();
    const int seat = movingSeat(game, action);
    Player& moved = game.players[seat];

    switch (action.kind) {
    case ActionKind::Drive:
    case ActionKind::ShuttleFlight:
        moved.city = action.target;
        break;

    case ActionKind::DirectFlight:
        removeCard(player.hand, cityCard(action.target));
        game.playerDiscard.push_back(cityCard(action.target));
        moved.city = action.target;
        break;

    case ActionKind::CharterFlight: {
        const PlayerCard spent = cityCard(moved.city);
        removeCard(player.hand, spent);
        game.playerDiscard.push_back(spent);
        moved.city = action.target;
        break;
    }

    case ActionKind::BuildStation: {
        const City here = player.city;
        const Reason placed = placeStation(game, here, action.target);
        if (placed != Reason::Ok)
            return placed;
        if (player.role != Role::Operations) {
            removeCard(player.hand, cityCard(here));
            game.playerDiscard.push_back(cityCard(here));
        }
        break;
    }

    case ActionKind::Treat: {
        const bool sweep = player.role == Role::Medic
                        || game.cures[static_cast<int>(action.colour)] != CureState::None;
        removeCubes(game, player.city, action.colour, sweep ? kCubesPerCity : 1);
        break;
    }

    case ActionKind::ShareGive:
        removeCard(player.hand, action.card);
        game.players[action.otherSeat].hand.push_back(action.card);
        break;

    case ActionKind::ShareTake:
        removeCard(game.players[action.otherSeat].hand, action.card);
        player.hand.push_back(action.card);
        break;

    case ActionKind::DiscoverCure:
        for (PlayerCard card : action.cure) {
            removeCard(player.hand, card);
            game.playerDiscard.push_back(card);
        }
        game.cures[static_cast<int>(action.colour)] = CureState::Cured;
        checkEradication(game, action.colour);
        medicSweep(game);
        checkVictory(game);
        break;

    case ActionKind::OperationsFlight:
        removeCard(player.hand, action.card);
        game.playerDiscard.push_back(action.card);
        player.operationsFlightUsed = true;
        moved.city = action.target;
        break;

    case ActionKind::PlannerTake:
        removeCard(game.playerDiscard, action.card);
        player.storedEvent = action.card;
        break;

    case ActionKind::Pass:
        game.actionsLeft = 1;   // the decrement below ends the action phase
        break;
    }

    medicSweep(game);
    if (game.over())
        return Reason::Ok;

    --game.actionsLeft;
    if (game.actionsLeft == 0)
        endActions(game);
    continueTurn(game);
    return Reason::Ok;
}

// Every action handed out here is complete: the pawn is named even where it
// could only be the player at turn, and an action that spends a card carries
// that card. check() and apply() would manage without -- they derive both from
// the game -- but the UI, the log and the network protocol all read the Action
// on its own, and a half filled one silently turns into a different move.
std::vector<Action> legalActions(const Game& game)
{
    std::vector<Action> actions;
    if (game.over() || game.phase != Phase::Actions)
        return actions;

    const Player& player = game.current();
    const bool dispatcher = player.role == Role::Dispatcher;
    const int seats = static_cast<int>(game.players.size());

    const auto offer = [&](Action action) {
        if (check(game, action) == Reason::Ok)
            actions.push_back(std::move(action));
    };

    for (int seat = 0; seat < seats; ++seat) {
        if (seat != game.atTurn && !dispatcher)
            continue;
        const City from = game.players[seat].city;
        const std::uint8_t pawn = static_cast<std::uint8_t>(seat);

        for (City next : neighbours(from)) {
            Action drive;
            drive.kind = ActionKind::Drive;
            drive.pawn = pawn;
            drive.target = next;
            offer(drive);
        }
        for (PlayerCard card : player.hand) {
            if (!card.isCity())
                continue;
            Action direct;
            direct.kind = ActionKind::DirectFlight;
            direct.pawn = pawn;
            direct.target = card.city();
            direct.card = card;
            offer(direct);
        }
        if (hasCard(player.hand, cityCard(from))) {
            for (int id = 0; id < kCities; ++id) {
                Action charter;
                charter.kind = ActionKind::CharterFlight;
                charter.pawn = pawn;
                charter.target = City(static_cast<std::uint8_t>(id));
                charter.card = cityCard(from);
                offer(charter);
            }
        }
        if (game.hasStation(from)) {
            for (int id = 0; id < kCities; ++id) {
                if (!game.hasStation(City(static_cast<std::uint8_t>(id))))
                    continue;
                Action shuttle;
                shuttle.kind = ActionKind::ShuttleFlight;
                shuttle.pawn = pawn;
                shuttle.target = City(static_cast<std::uint8_t>(id));
                offer(shuttle);
            }
        }
        if (dispatcher) {
            for (int other = 0; other < seats; ++other) {
                if (other == seat)
                    continue;
                Action toPawn;
                toPawn.kind = ActionKind::Drive;
                toPawn.pawn = pawn;
                toPawn.target = game.players[other].city;
                offer(toPawn);
            }
        }
    }

    {
        Action build;
        build.kind = ActionKind::BuildStation;
        build.pawn = game.atTurn;
        if (player.role != Role::Operations)
            build.card = cityCard(player.city);
        offer(build);
        if (game.stationCount() >= kStations) {
            for (int id = 0; id < kCities; ++id) {
                if (!game.hasStation(City(static_cast<std::uint8_t>(id))))
                    continue;
                build.target = City(static_cast<std::uint8_t>(id));
                offer(build);
            }
        }
    }

    for (int colour = 0; colour < game.colourCount(); ++colour) {
        Action treat;
        treat.kind = ActionKind::Treat;
        treat.pawn = game.atTurn;
        treat.colour = Colour(colour);
        offer(treat);
    }

    for (int other = 0; other < seats; ++other) {
        if (other == game.atTurn)
            continue;
        for (PlayerCard card : player.hand) {
            Action give;
            give.kind = ActionKind::ShareGive;
            give.pawn = game.atTurn;
            give.otherSeat = static_cast<std::uint8_t>(other);
            give.card = card;
            offer(give);
        }
        for (PlayerCard card : game.players[other].hand) {
            Action take;
            take.kind = ActionKind::ShareTake;
            take.pawn = game.atTurn;
            take.otherSeat = static_cast<std::uint8_t>(other);
            take.card = card;
            offer(take);
        }
    }

    // one canonical set per curable colour; which cards are spent hardly matters
    for (int colour = 0; colour < game.colourCount(); ++colour) {
        Action cure;
        cure.kind = ActionKind::DiscoverCure;
        cure.pawn = game.atTurn;
        cure.colour = Colour(colour);
        for (PlayerCard card : player.hand) {
            if (card.isCity() && card.colour() == Colour(colour)
                && static_cast<int>(cure.cure.size()) < cardsForCure(player.role))
                cure.cure.push_back(card);
        }
        offer(cure);
    }

    if (player.role == Role::Operations && !player.operationsFlightUsed) {
        for (PlayerCard card : player.hand) {
            if (!card.isCity())
                continue;
            for (int id = 0; id < kCities; ++id) {
                Action flight;
                flight.kind = ActionKind::OperationsFlight;
                flight.pawn = game.atTurn;
                flight.card = card;
                flight.target = City(static_cast<std::uint8_t>(id));
                offer(flight);
            }
        }
    }

    if (player.role == Role::Planner) {
        for (PlayerCard card : game.playerDiscard) {
            Action take;
            take.kind = ActionKind::PlannerTake;
            take.pawn = game.atTurn;
            take.card = card;
            offer(take);
        }
    }

    Action pass;
    pass.kind = ActionKind::Pass;
    pass.pawn = game.atTurn;
    offer(pass);
    return actions;
}

// --- events ----------------------------------------------------------------

Reason check(const Game& game, const EventPlay& play)
{
    if (game.over())
        return Reason::GameOver;
    if (!knownSeat(game, play.seat))
        return Reason::NoSuchSeat;

    const Player& player = game.players[play.seat];
    const PlayerCard card = eventCard(play.event);
    if (play.fromRoleCard) {
        if (player.role != Role::Planner || player.storedEvent != card)
            return Reason::EventNotHeld;
    } else if (!hasCard(player.hand, card)) {
        return Reason::EventNotHeld;
    }

    switch (play.event) {
    case Event::QuietNight:
        return Reason::Ok;
    case Event::Forecast: {
        if (static_cast<int>(game.infectionDeck.size()) < kForecastCards)
            return Reason::WrongPhase;
        if (static_cast<int>(play.order.size()) != kForecastCards)
            return Reason::UnknownCity;
        InfectionCards top(game.infectionDeck.end() - kForecastCards, game.infectionDeck.end());
        InfectionCards wanted = play.order;
        const auto byId = [](City a, City b) { return a.id < b.id; };
        std::sort(top.begin(), top.end(), byId);
        std::sort(wanted.begin(), wanted.end(), byId);
        return top == wanted ? Reason::Ok : Reason::UnknownCity;
    }
    case Event::Grant:
        if (!play.city.valid())
            return Reason::UnknownCity;
        if (game.hasStation(play.city))
            return Reason::StationAlreadyHere;
        if (game.stationCount() >= kStations)
            return Reason::NoStationLeft;
        return Reason::Ok;
    case Event::Airlift:
        if (!knownSeat(game, play.pawn))
            return Reason::NoSuchSeat;
        if (!play.city.valid())
            return Reason::UnknownCity;
        if (game.players[play.pawn].city == play.city)
            return Reason::SameCity;
        return Reason::Ok;
    case Event::Resilient:
        if (std::find(game.infectionDiscard.begin(), game.infectionDiscard.end(), play.removed)
            == game.infectionDiscard.end())
            return Reason::UnknownCity;
        return Reason::Ok;
    }
    return Reason::NotAnEventCard;
}

Reason apply(Game& game, const EventPlay& play, std::mt19937& rng)
{
    (void)rng;
    const Reason reason = check(game, play);
    if (reason != Reason::Ok)
        return reason;

    Player& player = game.players[play.seat];
    const PlayerCard card = eventCard(play.event);

    switch (play.event) {
    case Event::QuietNight:
        game.quietNight = true;
        break;
    case Event::Forecast:
        std::copy(play.order.begin(), play.order.end(), game.infectionDeck.end() - kForecastCards);
        break;
    case Event::Grant:
        game.stations.set(play.city.id);
        break;
    case Event::Airlift:
        game.players[play.pawn].city = play.city;
        medicSweep(game);
        break;
    case Event::Resilient: {
        const auto it = std::find(game.infectionDiscard.begin(), game.infectionDiscard.end(), play.removed);
        game.infectionDiscard.erase(it);
        game.removedFromGame.push_back(play.removed);
        break;
    }
    }

    // The Planner's stored card leaves the game, a card from hand is discarded.
    if (play.fromRoleCard)
        player.storedEvent = PlayerCard();
    else {
        removeCard(player.hand, card);
        game.playerDiscard.push_back(card);
    }

    // A card played to get under the limit can end the discard phase.
    if (game.phase == Phase::Discard)
        continueTurn(game);
    return Reason::Ok;
}

// --- phase steps -----------------------------------------------------------

void endActions(Game& game)
{
    if (game.over())
        return;
    game.actionsLeft = 0;
    game.drawsLeft = kDrawsPerTurn;
    game.phase = Phase::Draw;
}

void drawPlayerCard(Game& game, std::mt19937& rng)
{
    if (game.over() || game.phase != Phase::Draw || game.drawsLeft == 0)
        return;

    if (game.playerDeck.empty()) {
        game.outcome = Outcome::LostCards;
        game.phase = Phase::Over;
        return;
    }

    const PlayerCard card = game.playerDeck.back();
    game.playerDeck.pop_back();
    --game.drawsLeft;

    if (card.isEpidemic()) {
        game.playerDiscard.push_back(card);
        resolveEpidemic(game, rng);
    } else {
        game.current().hand.push_back(card);
    }
    continueTurn(game);
}

Reason discard(Game& game, int seat, PlayerCard card)
{
    if (game.over())
        return Reason::GameOver;
    if (game.phase != Phase::Discard || seat != game.discardingPlayer)
        return Reason::WrongPhase;
    if (!removeCard(game.players[seat].hand, card))
        return Reason::CardNotInHand;

    game.playerDiscard.push_back(card);
    continueTurn(game);
    return Reason::Ok;
}

void infectStep(Game& game)
{
    if (game.over() || game.phase != Phase::Infect)
        return;
    if (game.infectionsLeft == 0 || game.infectionDeck.empty()) {
        endTurn(game);
        return;
    }

    const City city = game.infectionDeck.back();
    game.infectionDeck.pop_back();
    game.infectionDiscard.push_back(city);
    placeCube(game, city, city.colour());
    --game.infectionsLeft;

    if (!game.over() && game.infectionsLeft == 0)
        endTurn(game);
}

void endTurn(Game& game)
{
    if (game.over())
        return;
    game.atTurn = static_cast<std::uint8_t>((game.atTurn + 1) % game.players.size());
    game.actionsLeft = kActionsPerTurn;
    game.drawsLeft = 0;
    game.infectionsLeft = 0;
    game.discardingPlayer = 0xFF;
    game.current().operationsFlightUsed = false;
    game.phase = Phase::Actions;
}

} // namespace seuche
