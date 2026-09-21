// Plays whole games through, thousands of them, and checks after every single
// step that the state still makes sense. This is the net that catches what the
// hand written positions in test_rules.cpp do not: rules that only misbehave
// once a real game has moved the board somewhere odd.
//
// Two things it does beyond replaying moves:
//
//   * Conservation. Cubes, player cards and infection cards are physical
//     objects; none may be created or lost. A rule that forgets to discard is
//     invisible in a single position and fatal over a game.
//   * Completeness of legalActions(). Every action check() accepts must also be
//     offered by legalActions(), because the UI only ever shows what
//     legalActions() returns. An action that is legal but never listed is a
//     move the player cannot make -- it looks like a broken card, not a broken
//     list.
//
// It also counts how often each action kind and each event was actually played,
// and fails if one of them never happens in a few thousand games: a move nobody
// can reach is as broken as one that is refused.

#include "../src/core/Rules.h"
#include "../src/core/Setup.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <map>
#include <random>
#include <cstdlib>
#include <string>
#include <vector>

using namespace seuche;

static int failures = 0;
static std::string context;    // seed / turn, printed with every failure
static std::string lastMove;   // what was applied just before the check
static bool stopAtFirst = false;

static void check(bool ok, const std::string& what)
{
    if (ok)
        return;
    ++failures;
    if (failures <= 40)
        std::printf("FAIL %s [%s]: %s\n", context.c_str(), lastMove.c_str(), what.c_str());
    else if (failures == 41)
        std::printf("... further failures suppressed\n");
}

// --- invariants ------------------------------------------------------------

static void checkConservation(const Game& game, const std::map<int, int>& deckAtStart,
                              int infectionTotal)
{
    for (int c = 0; c < game.colourCount(); ++c) {
        const Colour colour = Colour(c);
        const int inPlay = game.totalCubes(colour) + game.supply[c];
        check(inPlay == cubesInBox(colour),
              "cube conservation for colour " + std::to_string(c) + ": " +
                  std::to_string(inPlay) + " != " + std::to_string(cubesInBox(colour)));
    }

    std::map<int, int> seen;
    for (const PlayerCard& card : game.playerDeck)
        ++seen[card.id];
    for (const PlayerCard& card : game.playerDiscard)
        ++seen[card.id];
    for (const Player& player : game.players) {
        for (const PlayerCard& card : player.hand)
            ++seen[card.id];
        if (player.storedEvent.valid())
            ++seen[player.storedEvent.id];
    }
    if (seen != deckAtStart) {
        std::string diff;
        for (const auto& entry : deckAtStart) {
            const auto it = seen.find(entry.first);
            const int now = it == seen.end() ? 0 : it->second;
            if (now != entry.second)
                diff += " card " + std::to_string(entry.first) + ": " +
                        std::to_string(entry.second) + "->" + std::to_string(now);
        }
        for (const auto& entry : seen)
            if (deckAtStart.find(entry.first) == deckAtStart.end())
                diff += " card " + std::to_string(entry.first) + ": new x" +
                        std::to_string(entry.second);
        check(false, "player cards are not conserved:" + diff);
        if (stopAtFirst)
            std::exit(1);
    }

    const int infectionNow = static_cast<int>(game.infectionDeck.size() +
                                              game.infectionDiscard.size() +
                                              game.removedFromGame.size());
    check(infectionNow == infectionTotal,
          "infection cards: " + std::to_string(infectionNow) + " != " +
              std::to_string(infectionTotal));
}

static void checkState(const Game& game)
{
    check(game.stationCount() <= kStations,
          "more than " + std::to_string(kStations) + " research stations");
    check(game.atTurn < game.players.size(), "atTurn out of range");
    check(game.actionsLeft <= kActionsPerTurn, "actionsLeft above the maximum");
    check(game.infectionRateIndex < 7, "infection rate index out of range");
    check(game.outbreaks <= 8, "outbreak marker beyond 8");

    for (int city = 0; city < kCities; ++city)
        for (int c = 0; c < game.colourCount(); ++c)
            check(game.cubes[city][c] <= kCubesPerCity,
                  "city " + std::to_string(city) + " holds more than " +
                      std::to_string(kCubesPerCity) + " cubes");

    // Only the player who must discard may be over the limit, and only while
    // the game is waiting for that discard.
    for (std::size_t seat = 0; seat < game.players.size(); ++seat) {
        const bool discarding = game.phase == Phase::Discard &&
                                seat == game.discardingPlayer;
        if (!discarding)
            check(game.players[seat].hand.size() <= kHandLimit,
                  "seat " + std::to_string(seat) + " holds " +
                      std::to_string(game.players[seat].hand.size()) + " cards");
    }

    if (game.phase == Phase::Discard)
        check(game.discardingPlayer < game.players.size(),
              "Phase::Discard without a discarding player");

    check((game.phase == Phase::Over) == game.over(),
          "Phase::Over and Outcome disagree");
}

// --- candidate moves -------------------------------------------------------

// Everything except the cure cards: two cure actions that spend different
// subsets of the same hand are the same move as far as the player is concerned,
// and legalActions() is free to pick whichever subset it likes.
static bool sameMove(const Action& a, const Action& b)
{
    if (a.kind != b.kind)
        return false;
    if (a.kind == ActionKind::DiscoverCure)
        return a.colour == b.colour && a.pawn == b.pawn;
    if (a.kind == ActionKind::BuildStation)
        return a.pawn == b.pawn && a.target == b.target;   // the card is implied
    return a.pawn == b.pawn && a.target == b.target && a.otherSeat == b.otherSeat &&
           a.card == b.card && a.colour == b.colour;
}

static std::string describe(const Action& a)
{
    static const char* kinds[] = {"Drive", "DirectFlight", "CharterFlight", "ShuttleFlight",
                                  "BuildStation", "Treat", "ShareGive", "ShareTake",
                                  "DiscoverCure", "OperationsFlight", "PlannerTake", "Pass"};
    std::string s = kinds[static_cast<int>(a.kind)];
    s += " pawn=" + std::to_string(a.pawn);
    if (a.target.valid())
        s += " target=" + std::string(cityInfo(a.target).key);
    if (a.otherSeat != 0xFF)
        s += " other=" + std::to_string(a.otherSeat);
    if (a.card.valid())
        s += " card=" + std::to_string(a.card.id);
    if (a.kind == ActionKind::Treat || a.kind == ActionKind::DiscoverCure)
        s += " colour=" + std::to_string(static_cast<int>(a.colour));
    return s;
}

// Every action a player could plausibly try to take: the full cross product of
// kinds, pawns, cities and cards. check() then decides which of them are real.
static std::vector<Action> candidates(const Game& game)
{
    std::vector<Action> out;
    const int seats = static_cast<int>(game.players.size());

    for (int pawn = 0; pawn < seats; ++pawn) {
        const Player& mover = game.players[pawn];

        for (int city = 0; city < kCities; ++city) {
            const City target{static_cast<std::uint8_t>(city)};

            Action drive;
            drive.kind = ActionKind::Drive;
            drive.pawn = static_cast<std::uint8_t>(pawn);
            drive.target = target;
            out.push_back(drive);

            Action direct = drive;
            direct.kind = ActionKind::DirectFlight;
            direct.card = cityCard(target);
            out.push_back(direct);

            Action charter = drive;
            charter.kind = ActionKind::CharterFlight;
            charter.card = cityCard(mover.city);
            out.push_back(charter);

            Action shuttle = drive;
            shuttle.kind = ActionKind::ShuttleFlight;
            out.push_back(shuttle);

            // The Operations Expert spends any city card, so every card in the
            // hand at turn is a separate move.
            for (const PlayerCard& card : game.current().hand) {
                if (!card.isCity())
                    continue;
                Action ops = drive;
                ops.kind = ActionKind::OperationsFlight;
                ops.card = card;
                out.push_back(ops);
            }

            // Moving a pawn onto another pawn is the Dispatcher's other move.
            for (int other = 0; other < seats; ++other) {
                Action onto = drive;
                onto.target = City();
                onto.otherSeat = static_cast<std::uint8_t>(other);
                out.push_back(onto);
            }
        }
    }

    // A station is built where the pawn stands; target names the station to
    // take away, and only means anything once all six are on the board. Below
    // that it is ignored, so naming one is the same move as naming none and
    // legalActions() is right to list only the plain one.
    Action build;
    build.kind = ActionKind::BuildStation;
    build.pawn = game.atTurn;
    build.card = game.current().role == Role::Operations ? PlayerCard()
                                                         : cityCard(game.current().city);
    out.push_back(build);
    if (game.stationCount() >= kStations) {
        for (int city = 0; city < kCities; ++city) {
            Action moving = build;
            moving.target = City{static_cast<std::uint8_t>(city)};
            out.push_back(moving);
        }
    }

    for (int c = 0; c < game.colourCount(); ++c) {
        Action treat;
        treat.kind = ActionKind::Treat;
        treat.pawn = game.atTurn;
        treat.colour = Colour(c);
        out.push_back(treat);

        // Cure: offer every prefix of the matching cards, so a hand that can
        // pay at all produces at least one candidate check() accepts.
        PlayerCards matching;
        for (const PlayerCard& card : game.current().hand)
            if (card.isCity() && card.colour() == Colour(c))
                matching.push_back(card);
        for (std::size_t take = 4; take <= matching.size() && take <= 6; ++take) {
            Action cure;
            cure.kind = ActionKind::DiscoverCure;
            cure.pawn = game.atTurn;
            cure.colour = Colour(c);
            cure.cure.assign(matching.begin(), matching.begin() + take);
            out.push_back(cure);
        }
    }

    for (int other = 0; other < seats; ++other) {
        if (other == game.atTurn)
            continue;
        for (const PlayerCard& card : game.current().hand) {
            Action give;
            give.kind = ActionKind::ShareGive;
            give.pawn = game.atTurn;
            give.otherSeat = static_cast<std::uint8_t>(other);
            give.card = card;
            out.push_back(give);
        }
        for (const PlayerCard& card : game.players[other].hand) {
            Action take;
            take.kind = ActionKind::ShareTake;
            take.pawn = game.atTurn;
            take.otherSeat = static_cast<std::uint8_t>(other);
            take.card = card;
            out.push_back(take);
        }
    }

    for (const PlayerCard& card : game.playerDiscard) {
        if (!card.isEvent())
            continue;
        Action planner;
        planner.kind = ActionKind::PlannerTake;
        planner.pawn = game.atTurn;
        planner.card = card;
        out.push_back(planner);
    }

    Action pass;
    pass.kind = ActionKind::Pass;
    pass.pawn = game.atTurn;
    out.push_back(pass);

    return out;
}

// Every event play anyone could attempt right now.
static std::vector<EventPlay> eventCandidates(const Game& game, std::mt19937& rng)
{
    std::vector<EventPlay> out;
    const int seats = static_cast<int>(game.players.size());

    for (int seat = 0; seat < seats; ++seat) {
        PlayerCards sources = game.players[seat].hand;
        const bool hasStored = game.players[seat].storedEvent.valid();
        if (hasStored)
            sources.push_back(game.players[seat].storedEvent);

        for (std::size_t i = 0; i < sources.size(); ++i) {
            const PlayerCard& card = sources[i];
            if (!card.isEvent())
                continue;

            EventPlay base;
            base.seat = static_cast<std::uint8_t>(seat);
            base.event = card.event();
            base.fromRoleCard = hasStored && i + 1 == sources.size();

            switch (card.event()) {
            case Event::QuietNight:
                out.push_back(base);
                break;
            case Event::Forecast: {
                if (game.infectionDeck.size() < 6)
                    break;
                EventPlay play = base;
                play.order.assign(game.infectionDeck.end() - 6, game.infectionDeck.end());
                std::shuffle(play.order.begin(), play.order.end(), rng);
                out.push_back(play);
                break;
            }
            case Event::Grant:
                for (int city = 0; city < kCities; ++city) {
                    EventPlay play = base;
                    play.city = City{static_cast<std::uint8_t>(city)};
                    out.push_back(play);
                }
                break;
            case Event::Airlift:
                for (int pawn = 0; pawn < seats; ++pawn) {
                    for (int city = 0; city < kCities; ++city) {
                        EventPlay play = base;
                        play.pawn = static_cast<std::uint8_t>(pawn);
                        play.city = City{static_cast<std::uint8_t>(city)};
                        out.push_back(play);
                    }
                }
                break;
            case Event::Resilient:
                for (const City& city : game.infectionDiscard) {
                    EventPlay play = base;
                    play.removed = city;
                    out.push_back(play);
                }
                break;
            }
        }
    }
    return out;
}

// --- the driver ------------------------------------------------------------

struct Stats {
    std::array<long, 12> actions = {};
    std::array<long, kBaseEvents> events = {};
    std::array<long, 5> outcomes = {};
    long games = 0;
    long turns = 0;
    long legalActionsEmpty = 0;
};

// Plays one game to the end. `deep` turns on the expensive cross check of
// legalActions() against check(), which is far too slow to run on every game.
static void playGame(unsigned seed, int seats, Difficulty difficulty, bool deep, Stats& stats)
{
    std::mt19937 rng(seed);

    SetupOptions options;
    options.players = seats;
    options.difficulty = difficulty;
    Game game = newGame(options, rng);

    std::map<int, int> deckAtStart;
    for (const PlayerCard& card : game.playerDeck)
        ++deckAtStart[card.id];
    for (const PlayerCard& card : game.playerDiscard)
        ++deckAtStart[card.id];
    for (const Player& player : game.players)
        for (const PlayerCard& card : player.hand)
            ++deckAtStart[card.id];
    const int infectionTotal = static_cast<int>(game.infectionDeck.size() +
                                                game.infectionDiscard.size() +
                                                game.removedFromGame.size());

    checkState(game);
    checkConservation(game, deckAtStart, infectionTotal);

    // A game cannot outlast its deck, so anything past this is a loop.
    const long limit = 20000;
    long steps = 0;

    while (!game.over() && steps++ < limit) {
        context = "seed " + std::to_string(seed) + " seats " + std::to_string(seats) +
                  " step " + std::to_string(steps);

        // Events may be played at almost any moment, so try one at every step
        // rather than only during the action phase.
        if (rng() % 6 == 0) {
            std::vector<EventPlay> plays = eventCandidates(game, rng);
            std::vector<EventPlay> legal;
            for (const EventPlay& play : plays)
                if (check(game, play) == Reason::Ok)
                    legal.push_back(play);
            if (!legal.empty()) {
                const EventPlay& play = legal[rng() % legal.size()];
                const Event event = play.event;
                lastMove = std::string("event ") + eventName(event) +
                           (play.fromRoleCard ? " (from the role card)" : "") +
                           " by seat " + std::to_string(play.seat);
                const bool leavesGame = play.fromRoleCard;
                const PlayerCard spent = eventCard(event);
                const Reason reason = apply(game, play, rng);
                check(reason == Reason::Ok,
                      "a legal event was refused: event " +
                          std::to_string(static_cast<int>(event)) + " -> " +
                          reasonText(reason));
                if (reason == Reason::Ok) {
                    ++stats.events[static_cast<int>(event)];
                    // The Planner's stored card is removed from the game when
                    // it is played, so it is gone for good and the count of
                    // cards in circulation drops by one.
                    if (leavesGame && --deckAtStart[spent.id] == 0)
                        deckAtStart.erase(spent.id);
                }
                checkState(game);
                checkConservation(game, deckAtStart, infectionTotal);
            }
        }
        if (game.over())
            break;

        switch (game.phase) {
        case Phase::Actions: {
            std::vector<Action> legal = legalActions(game);
            if (legal.empty()) {
                ++stats.legalActionsEmpty;
                check(false, "legalActions() is empty during the action phase");
                endActions(game);
                break;
            }

            for (const Action& action : legal) {
                const Reason reason = check(game, action);
                check(reason == Reason::Ok,
                      "legalActions() offered a move check() refuses: " +
                          describe(action) + " -> " + reasonText(reason));
            }

            if (deep) {
                // The other direction, and the one that hides real bugs: a move
                // check() accepts but legalActions() never lists is a move no
                // player can reach through the interface.
                for (const Action& candidate : candidates(game)) {
                    if (check(game, candidate) != Reason::Ok)
                        continue;
                    const bool listed =
                        std::any_of(legal.begin(), legal.end(),
                                    [&](const Action& a) { return sameMove(a, candidate); });
                    check(listed, "legal but not offered by legalActions(): " +
                                      describe(candidate));
                }
            }

            // A purely random player never gathers five cards of one colour,
            // so it never cures and never wins -- and then the whole victory
            // half of the rules goes untested. This one grabs a cure the moment
            // one is legal, and otherwise plays at random.
            std::vector<std::size_t> thrifty;   // moves that spend no card
            std::size_t pick = legal.size();
            for (std::size_t i = 0; i < legal.size(); ++i) {
                if (legal[i].kind == ActionKind::DiscoverCure) {
                    pick = i;
                    break;
                }
                switch (legal[i].kind) {
                case ActionKind::DirectFlight:
                case ActionKind::CharterFlight:
                case ActionKind::OperationsFlight:
                case ActionKind::ShareGive:
                case ActionKind::Pass:
                    break;
                default:
                    thrifty.push_back(i);
                    break;
                }
            }
            if (pick == legal.size()) {
                // Mostly keep the cards: a player who flies on every card never
                // holds five of a colour, never cures, and never wins, and then
                // victory goes untested however many games are played.
                if (!thrifty.empty() && rng() % 10 != 0)
                    pick = thrifty[rng() % thrifty.size()];
                else
                    pick = rng() % legal.size();
            }
            const Action& action = legal[pick];
            const ActionKind kind = action.kind;
            lastMove = describe(action) + " by seat " + std::to_string(game.atTurn);
            const Reason reason = apply(game, action, rng);
            check(reason == Reason::Ok,
                  "apply() refused a legal action: " + describe(action) + " -> " +
                      reasonText(reason));
            if (reason == Reason::Ok)
                ++stats.actions[static_cast<int>(kind)];

            if (game.phase == Phase::Actions && game.actionsLeft == 0)
                endActions(game);
            break;
        }
        case Phase::Draw:
            lastMove = "drawPlayerCard";
            drawPlayerCard(game, rng);
            break;
        case Phase::Discard: {
            const int seat = game.discardingPlayer;
            check(seat >= 0 && seat < static_cast<int>(game.players.size()),
                  "Phase::Discard names no valid seat");
            if (seat < 0 || seat >= static_cast<int>(game.players.size()))
                return;
            const PlayerCards& hand = game.players[seat].hand;
            check(!hand.empty(), "asked to discard from an empty hand");
            if (hand.empty())
                return;
            // Throw away the colour the hand holds least of, so hands drift
            // towards a curable set instead of staying an even mush.
            std::array<int, kColourSlots> owned = {};
            for (const PlayerCard& c : hand)
                if (c.isCity())
                    ++owned[static_cast<int>(c.colour())];
            std::size_t worst = 0;
            int worstScore = 1000;
            for (std::size_t i = 0; i < hand.size(); ++i) {
                const int score = hand[i].isCity()
                                      ? owned[static_cast<int>(hand[i].colour())]
                                      : 100;   // keep event cards, they are rarer
                if (score < worstScore) {
                    worstScore = score;
                    worst = i;
                }
            }
            const PlayerCard card = hand[worst];
            lastMove = "discard card " + std::to_string(card.id) +
                       " from seat " + std::to_string(seat);
            const Reason reason = discard(game, seat, card);
            check(reason == Reason::Ok,
                  std::string("discard() refused a card from the hand: ") + reasonText(reason));
            break;
        }
        case Phase::Infect:
            lastMove = "infectStep";
            infectStep(game);
            break;
        case Phase::Over:
            break;
        }

        checkState(game);
        checkConservation(game, deckAtStart, infectionTotal);
        ++stats.turns;
    }

    check(steps < limit, "the game did not end within " + std::to_string(limit) + " steps");
    ++stats.games;
    ++stats.outcomes[static_cast<int>(game.outcome)];
}

int main(int argc, char** argv)
{
    stopAtFirst = argc > 1 && std::string(argv[1]) == "--first";
    std::string problem;
    check(mapIsSane(&problem), "the map is not sane: " + problem);

    Stats stats;

    // Many fast games for the invariants ...
    for (unsigned seed = 1; seed <= 2000; ++seed) {
        const int seats = 2 + static_cast<int>(seed % 3);
        const Difficulty difficulty = Difficulty(seed % 3);
        playGame(seed, seats, difficulty, false, stats);
    }
    // ... and a few slow ones for the legalActions() cross check.
    for (unsigned seed = 90001; seed <= 90030; ++seed)
        playGame(seed, 2 + static_cast<int>(seed % 3), Difficulty::Normal, true, stats);

    context = "summary";

    static const char* kinds[] = {"Drive", "DirectFlight", "CharterFlight", "ShuttleFlight",
                                  "BuildStation", "Treat", "ShareGive", "ShareTake",
                                  "DiscoverCure", "OperationsFlight", "PlannerTake", "Pass"};
    std::printf("\n%ld games, %ld steps\n\n", stats.games, stats.turns);
    std::printf("action                 played\n");
    for (int i = 0; i < 12; ++i) {
        std::printf("  %-18s %8ld%s\n", kinds[i], stats.actions[i],
                    stats.actions[i] == 0 ? "   <-- never" : "");
        check(stats.actions[i] > 0,
              std::string("action ") + kinds[i] + " was never playable in any game");
    }

    std::printf("\nevent                  played\n");
    for (int i = 0; i < kBaseEvents; ++i) {
        std::printf("  %-18s %8ld%s\n", eventName(Event(i)), stats.events[i],
                    stats.events[i] == 0 ? "   <-- never" : "");
        check(stats.events[i] > 0,
              std::string("event ") + eventName(Event(i)) + " was never playable in any game");
    }

    static const char* outcomes[] = {"Running", "Won", "LostOutbreaks", "LostCubes", "LostCards"};
    std::printf("\noutcome                 games\n");
    for (int i = 0; i < 5; ++i)
        std::printf("  %-18s %8ld\n", outcomes[i], stats.outcomes[i]);
    check(stats.outcomes[0] == 0, "a game ended still Running");

    std::printf("\n%s (%d failures)\n", failures ? "FAILED" : "all checks pass", failures);
    return failures ? 1 : 0;
}
