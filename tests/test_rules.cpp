// Engine tests. Positions are built by hand instead of by dealing, so every case
// is exactly the rule it is named after.
#include "../src/core/Rules.h"
#include "../src/core/Setup.h"

#include <cstdio>
#include <string>

using namespace seuche;

static int failures = 0;

static void check(bool ok, const std::string& what)
{
    if (!ok) {
        std::printf("FAIL  %s\n", what.c_str());
        ++failures;
    }
}

// An empty board with `seats` players in Atlanta, nothing dealt.
static Game bare(int seats = 2, Role first = Role::Scientist)
{
    Game game;
    game.players.resize(seats);
    for (int seat = 0; seat < seats; ++seat) {
        game.players[seat].role = seat == 0 ? first : Role::Researcher;
        game.players[seat].city = kStartCity;
    }
    game.players[0].role = first;
    game.stations.set(kStartCity.id);
    game.supply.fill(0);
    for (int colour = 0; colour < kBaseColours; ++colour)
        game.supply[colour] = kCubesPerColour;
    game.cures.fill(CureState::None);
    game.phase = Phase::Actions;
    game.actionsLeft = kActionsPerTurn;
    return game;
}

static void put(Game& game, const char* key, Colour colour, int cubes)
{
    const City city = cityByKey(key);
    game.cubes[city.id][static_cast<int>(colour)] = static_cast<std::uint8_t>(cubes);
    game.supply[static_cast<int>(colour)] -= static_cast<std::uint8_t>(cubes);
}

static int cubesAt(const Game& game, const char* key, Colour colour)
{
    return game.cubes_(cityByKey(key), colour);
}

static bool conserved(const Game& game)
{
    for (int colour = 0; colour < game.colourCount(); ++colour) {
        const int expected = cubesInBox(Colour(colour));
        if (game.supply[colour] + game.totalCubes(Colour(colour)) != expected)
            return false;
    }
    return true;
}

static void testMovement()
{
    std::mt19937 rng(1);
    Game game = bare();
    Action action;

    action.kind = ActionKind::Drive;
    action.target = cityByKey("washington");
    check(apply(game, action, rng) == Reason::Ok, "drive to a neighbour");
    check(game.current().city == cityByKey("washington"), "pawn moved");
    check(game.actionsLeft == 3, "one action spent");

    action.target = cityByKey("tokio");
    check(check(game, action) == Reason::NotAdjacent, "no drive across the map");

    // direct flight needs the card of the destination
    game.current().hand.push_back(cityCard(cityByKey("tokio")));
    action.kind = ActionKind::DirectFlight;
    check(apply(game, action, rng) == Reason::Ok, "direct flight");
    check(game.current().city == cityByKey("tokio"), "flown to Tokio");
    check(hasCard(game.playerDiscard, cityCard(cityByKey("tokio"))), "card discarded");

    // charter flight spends the card of the city you stand in
    game.current().hand.push_back(cityCard(cityByKey("tokio")));
    action.kind = ActionKind::CharterFlight;
    action.target = cityByKey("kairo");
    check(apply(game, action, rng) == Reason::Ok, "charter flight");
    check(game.current().city == cityByKey("kairo"), "flown to Kairo");

    // shuttle needs a station on both ends
    action.kind = ActionKind::ShuttleFlight;
    action.target = kStartCity;
    check(check(game, action) == Reason::NoStationHere, "shuttle without a station");
}

static void testStationsAndCure()
{
    std::mt19937 rng(2);
    Game game = bare(2, Role::Scientist);
    Player& player = game.players[0];
    player.city = cityByKey("paris");
    player.hand.push_back(cityCard(cityByKey("paris")));

    Action build;
    build.kind = ActionKind::BuildStation;
    check(apply(game, build, rng) == Reason::Ok, "build a station");
    check(game.hasStation(cityByKey("paris")), "station in Paris");
    check(game.stationCount() == 2, "two stations");

    // the scientist cures with four cards; one blue cube keeps it from being
    // eradicated in the same breath
    put(game, "london", Colour::Blue, 1);
    const char* blue[] = {"london", "madrid", "essen", "mailand"};
    Action cure;
    cure.kind = ActionKind::DiscoverCure;
    cure.colour = Colour::Blue;
    for (const char* key : blue) {
        player.hand.push_back(cityCard(cityByKey(key)));
        cure.cure.push_back(cityCard(cityByKey(key)));
    }
    check(check(game, cure) == Reason::Ok, "four cards are enough for the scientist");
    check(apply(game, cure, rng) == Reason::Ok, "cure discovered");
    check(game.cures[0] == CureState::Cured, "blue is cured");
    check(check(game, cure) == Reason::AlreadyCured, "no second blue cure");

    // it only becomes eradicated once the last cube is gone
    Game clean = game;
    removeCubes(clean, cityByKey("london"), Colour::Blue, 1);
    check(clean.cures[0] == CureState::Eradicated, "eradicated with the last cube");

    // five cards for everyone else
    Game other = bare(2, Role::Researcher);
    other.players[0].city = kStartCity;
    Action cure5;
    cure5.kind = ActionKind::DiscoverCure;
    cure5.colour = Colour::Blue;
    for (const char* key : blue) {
        other.players[0].hand.push_back(cityCard(cityByKey(key)));
        cure5.cure.push_back(cityCard(cityByKey(key)));
    }
    check(check(other, cure5) == Reason::NotEnoughCards, "four cards are not enough otherwise");
}

static void testTreatAndEradication()
{
    std::mt19937 rng(3);
    Game game = bare(2, Role::Researcher);
    game.players[0].city = cityByKey("kairo");
    put(game, "kairo", Colour::Black, 3);

    Action treat;
    treat.kind = ActionKind::Treat;
    treat.colour = Colour::Black;
    check(apply(game, treat, rng) == Reason::Ok, "treat one cube");
    check(cubesAt(game, "kairo", Colour::Black) == 2, "one cube gone");

    game.cures[static_cast<int>(Colour::Black)] = CureState::Cured;
    check(apply(game, treat, rng) == Reason::Ok, "treat a cured disease");
    check(cubesAt(game, "kairo", Colour::Black) == 0, "all cubes gone at once");
    check(game.cures[static_cast<int>(Colour::Black)] == CureState::Eradicated, "eradicated");
    check(conserved(game), "cubes conserved");

    // the medic clears cured cubes just by arriving
    Game medic = bare(2, Role::Medic);
    medic.cures[static_cast<int>(Colour::Blue)] = CureState::Cured;
    put(medic, "washington", Colour::Blue, 3);
    Action drive;
    drive.kind = ActionKind::Drive;
    drive.target = cityByKey("washington");
    check(apply(medic, drive, rng) == Reason::Ok, "medic drives");
    check(cubesAt(medic, "washington", Colour::Blue) == 0, "medic swept the city");

    // and keeps them out while she stands there
    placeCube(medic, cityByKey("washington"), Colour::Blue);
    check(cubesAt(medic, "washington", Colour::Blue) == 0, "medic blocks cured cubes");
}

static void testOutbreak()
{
    std::mt19937 rng(4);
    Game game = bare();
    put(game, "atlanta", Colour::Blue, 3);

    placeCube(game, cityByKey("atlanta"), Colour::Blue);
    check(game.outbreaks == 1, "one outbreak");
    check(cubesAt(game, "atlanta", Colour::Blue) == 3, "the city keeps three cubes");
    check(cubesAt(game, "chicago", Colour::Blue) == 1, "neighbour infected");
    check(cubesAt(game, "washington", Colour::Blue) == 1, "neighbour infected");
    check(cubesAt(game, "miami", Colour::Blue) == 1, "neighbour infected");
    check(conserved(game), "cubes conserved");

    // chain reaction: every city bursts at most once
    Game chain = bare();
    put(chain, "atlanta", Colour::Blue, 3);
    put(chain, "washington", Colour::Blue, 3);
    placeCube(chain, cityByKey("atlanta"), Colour::Blue);
    check(chain.outbreaks == 2, "two outbreaks in the chain");
    check(cubesAt(chain, "atlanta", Colour::Blue) == 3, "no cube returns to the starter");
    check(cubesAt(chain, "montreal", Colour::Blue) == 1, "the chain reached Montreal");
    check(conserved(chain), "cubes conserved in the chain");

    // the quarantine specialist keeps her city and its neighbours clean
    Game guard = bare(2, Role::Quarantine);
    guard.players[0].city = cityByKey("miami");
    placeCube(guard, cityByKey("miami"), Colour::Yellow);
    placeCube(guard, cityByKey("atlanta"), Colour::Yellow);
    placeCube(guard, cityByKey("tokio"), Colour::Red);
    check(cubesAt(guard, "miami", Colour::Yellow) == 0, "shielded city");
    check(cubesAt(guard, "atlanta", Colour::Yellow) == 0, "shielded neighbour");
    check(cubesAt(guard, "tokio", Colour::Red) == 1, "elsewhere unaffected");

    // eight outbreaks lose the game
    Game doomed = bare();
    doomed.outbreaks = 7;
    put(doomed, "tokio", Colour::Red, 3);
    placeCube(doomed, cityByKey("tokio"), Colour::Red);
    check(doomed.outcome == Outcome::LostOutbreaks, "eight outbreaks lose");

    // so does an empty cube supply
    Game empty = bare();
    empty.supply[static_cast<int>(Colour::Red)] = 0;
    placeCube(empty, cityByKey("tokio"), Colour::Red);
    check(empty.outcome == Outcome::LostCubes, "no cubes left loses");
}

static void testEpidemic()
{
    std::mt19937 rng(5);
    Game game = bare();
    game.infectionDeck = {cityByKey("lima"), cityByKey("kairo"), cityByKey("tokio")};
    game.infectionDiscard = {cityByKey("paris")};
    game.playerDeck = {kEpidemicCard};
    endActions(game);

    const int stepBefore = game.infectionRateIndex;
    drawPlayerCard(game, rng);
    check(game.infectionRateIndex == stepBefore + 1, "the rate marker moved on");
    check(cubesAt(game, "lima", Colour::Yellow) == 3, "the bottom card got three cubes");
    check(game.infectionDiscard.empty(), "the discard pile was shuffled back");
    check(game.infectionDeck.size() == 4, "deck holds the rest plus the intensified cards");
    check(game.infectionDeck.back() == cityByKey("lima") || game.infectionDeck.back() == cityByKey("paris"),
          "an intensified card lies on top");
    check(conserved(game), "cubes conserved");

    // an epidemic in a city that already holds cubes bursts instead of stacking
    Game burst = bare();
    burst.infectionDeck = {cityByKey("lima")};
    put(burst, "lima", Colour::Yellow, 1);
    burst.playerDeck = {kEpidemicCard};
    endActions(burst);
    drawPlayerCard(burst, rng);
    check(cubesAt(burst, "lima", Colour::Yellow) == 3, "capped at three");
    check(burst.outbreaks == 1, "the epidemic caused an outbreak");
}

static void testDrawAndInfect()
{
    std::mt19937 rng(6);
    Game game = bare();
    game.infectionDeck = {cityByKey("tokio"), cityByKey("kairo"), cityByKey("lima")};
    game.playerDeck = {cityCard(cityByKey("paris")), cityCard(cityByKey("london"))};

    endActions(game);
    check(game.phase == Phase::Draw, "draw phase after the actions");
    drawPlayerCard(game, rng);
    drawPlayerCard(game, rng);
    check(game.players[0].hand.size() == 2, "two cards drawn");
    check(game.phase == Phase::Infect, "infect phase follows");
    check(game.infectionsLeft == 2, "rate is two at the start");

    infectStep(game);
    check(cubesAt(game, "lima", Colour::Yellow) == 1, "top infection card resolved");
    infectStep(game);
    check(game.phase == Phase::Actions, "turn passed on");
    check(game.atTurn == 1, "next seat is at turn");
    check(game.actionsLeft == kActionsPerTurn, "actions refilled");

    // running out of player cards loses the game
    Game out = bare();
    endActions(out);
    drawPlayerCard(out, rng);
    check(out.outcome == Outcome::LostCards, "empty player deck loses");

    // over the hand limit the turn stops until someone discards
    Game full = bare();
    for (int card = 0; card < 7; ++card)
        full.players[0].hand.push_back(cityCard(City(static_cast<std::uint8_t>(card))));
    full.playerDeck = {cityCard(cityByKey("tokio")), cityCard(cityByKey("paris"))};
    full.infectionDeck = {cityByKey("lima"), cityByKey("kairo")};
    endActions(full);
    drawPlayerCard(full, rng);
    check(full.phase == Phase::Discard, "hand limit stops the turn");
    check(discard(full, 0, cityCard(cityByKey("paris"))) == Reason::Ok, "discarding works");
    check(full.phase == Phase::Draw, "the second card is still owed");
}

static void testEvents()
{
    std::mt19937 rng(7);

    // quiet night skips the next infection phase
    Game game = bare();
    game.players[0].hand.push_back(eventCard(Event::QuietNight));
    game.infectionDeck = {cityByKey("tokio"), cityByKey("kairo"), cityByKey("lima")};
    game.playerDeck = {cityCard(cityByKey("paris")), cityCard(cityByKey("london"))};
    EventPlay quiet;
    quiet.seat = 0;
    quiet.event = Event::QuietNight;
    check(apply(game, quiet, rng) == Reason::Ok, "quiet night played");
    endActions(game);
    drawPlayerCard(game, rng);
    drawPlayerCard(game, rng);
    check(game.phase == Phase::Actions && game.atTurn == 1, "infection phase skipped");
    check(game.infectionDeck.size() == 3, "no infection card turned");

    // airlift moves any pawn
    Game lift = bare(2);
    lift.players[1].hand.push_back(eventCard(Event::Airlift));
    EventPlay airlift;
    airlift.seat = 1;
    airlift.event = Event::Airlift;
    airlift.pawn = 0;
    airlift.city = cityByKey("sydney");
    check(apply(lift, airlift, rng) == Reason::Ok, "airlift played");
    check(lift.players[0].city == cityByKey("sydney"), "pawn airlifted");

    // government grant builds without a card
    Game grant = bare();
    grant.players[0].hand.push_back(eventCard(Event::Grant));
    EventPlay budget;
    budget.seat = 0;
    budget.event = Event::Grant;
    budget.city = cityByKey("hongkong");
    check(apply(grant, budget, rng) == Reason::Ok, "grant played");
    check(grant.hasStation(cityByKey("hongkong")), "station built for free");

    // resilient population takes a card out of the game
    Game tough = bare();
    tough.players[0].hand.push_back(eventCard(Event::Resilient));
    tough.infectionDiscard = {cityByKey("kairo"), cityByKey("lima")};
    EventPlay resilient;
    resilient.seat = 0;
    resilient.event = Event::Resilient;
    resilient.removed = cityByKey("kairo");
    check(apply(tough, resilient, rng) == Reason::Ok, "resilient population played");
    check(tough.infectionDiscard.size() == 1, "card left the discard pile");
    check(tough.removedFromGame.size() == 1, "card is out of the game");

    // forecast reorders the top six
    Game seer = bare();
    seer.players[0].hand.push_back(eventCard(Event::Forecast));
    for (int id = 0; id < 6; ++id)
        seer.infectionDeck.push_back(City(static_cast<std::uint8_t>(id)));
    EventPlay forecast;
    forecast.seat = 0;
    forecast.event = Event::Forecast;
    forecast.order = {City(5), City(4), City(3), City(2), City(1), City(0)};
    check(apply(seer, forecast, rng) == Reason::Ok, "forecast played");
    check(seer.infectionDeck.back() == City(0), "chosen card lies on top");

    forecast.order = {City(5), City(5), City(3), City(2), City(1), City(0)};
    check(check(seer, forecast) != Reason::Ok, "forecast must be a permutation");
}

static void testRoles()
{
    std::mt19937 rng(8);

    // the researcher hands over any card
    Game game = bare(2, Role::Researcher);
    game.players[1].role = Role::Scientist;
    game.players[0].hand.push_back(cityCard(cityByKey("tokio")));
    Action give;
    give.kind = ActionKind::ShareGive;
    give.otherSeat = 1;
    give.card = cityCard(cityByKey("tokio"));
    check(check(game, give) == Reason::Ok, "researcher gives any card");

    Game plain = bare(2, Role::Scientist);
    plain.players[0].hand.push_back(cityCard(cityByKey("tokio")));
    check(check(plain, give) == Reason::WrongShareCard, "others only the card of their city");

    // the dispatcher moves someone else
    Game dispatch = bare(2, Role::Dispatcher);
    Action drive;
    drive.kind = ActionKind::Drive;
    drive.pawn = 1;
    drive.target = cityByKey("chicago");
    check(apply(dispatch, drive, rng) == Reason::Ok, "dispatcher moves another pawn");
    check(dispatch.players[1].city == cityByKey("chicago"), "the other pawn moved");

    Game notDispatch = bare(2, Role::Scientist);
    check(check(notDispatch, drive) == Reason::NotYourPawn, "only the dispatcher may");

    // the operations expert builds for free and flies once per turn
    Game ops = bare(2, Role::Operations);
    ops.players[0].city = cityByKey("paris");
    Action build;
    build.kind = ActionKind::BuildStation;
    check(apply(ops, build, rng) == Reason::Ok, "operations builds without a card");
    check(ops.players[0].hand.empty(), "no card spent");

    ops.players[0].hand.push_back(cityCard(cityByKey("tokio")));
    Action flight;
    flight.kind = ActionKind::OperationsFlight;
    flight.card = cityCard(cityByKey("tokio"));
    flight.target = cityByKey("lima");
    check(apply(ops, flight, rng) == Reason::Ok, "operations flight");
    check(ops.players[0].city == cityByKey("lima"), "flown with any card");
    ops.players[0].hand.push_back(cityCard(cityByKey("kairo")));
    flight.card = cityCard(cityByKey("kairo"));
    flight.target = cityByKey("bagdad");
    check(check(ops, flight) != Reason::Ok, "only once per turn");

    // the contingency planner keeps an event card
    Game planner = bare(2, Role::Planner);
    planner.playerDiscard.push_back(eventCard(Event::Airlift));
    Action take;
    take.kind = ActionKind::PlannerTake;
    take.card = eventCard(Event::Airlift);
    check(apply(planner, take, rng) == Reason::Ok, "planner takes an event card");
    check(planner.players[0].storedEvent == eventCard(Event::Airlift), "card stored on the role");
    check(planner.playerDiscard.empty(), "card left the discard pile");

    EventPlay play;
    play.seat = 0;
    play.event = Event::Airlift;
    play.fromRoleCard = true;
    play.pawn = 1;
    play.city = cityByKey("sydney");
    check(apply(planner, play, rng) == Reason::Ok, "stored card can be played");
    check(!planner.players[0].storedEvent.valid(), "stored card is gone");
    check(planner.playerDiscard.empty(), "and left the game, not the discard pile");
}

static void testVictory()
{
    std::mt19937 rng(9);
    Game game = bare(2, Role::Scientist);
    for (int colour = 0; colour < kBaseColours - 1; ++colour)
        game.cures[colour] = CureState::Cured;

    Action cure;
    cure.kind = ActionKind::DiscoverCure;
    cure.colour = Colour::Red;
    for (int offset = 0; offset < 4; ++offset) {
        const PlayerCard card = cityCard(City(static_cast<std::uint8_t>(firstCityOf(Colour::Red) + offset)));
        game.players[0].hand.push_back(card);
        cure.cure.push_back(card);
    }
    check(apply(game, cure, rng) == Reason::Ok, "last cure discovered");
    check(game.outcome == Outcome::Won, "four cures win the game");
}

// Plays whole games with random legal moves. Nothing is asserted about the
// result — this is here to catch an engine that corrupts its own state.
static void testRandomGames()
{
    int won = 0, lost = 0;
    int byOutbreaks = 0, byCubes = 0, byCards = 0;
    for (int seed = 0; seed < 200; ++seed) {
        std::mt19937 rng(seed);
        SetupOptions options;
        options.players = 2 + seed % 3;
        Game game = newGame(options, rng);

        int steps = 0;
        while (!game.over() && steps++ < 20000) {
            switch (game.phase) {
            case Phase::Actions: {
                const std::vector<Action> actions = legalActions(game);
                check(!actions.empty(), "there is always a legal action");
                if (actions.empty())
                    return;
                const Action& action = actions[rng() % actions.size()];
                check(apply(game, action, rng) == Reason::Ok, "a legal action applies");
                break;
            }
            case Phase::Draw:
                drawPlayerCard(game, rng);
                break;
            case Phase::Discard: {
                Player& player = game.players[game.discardingPlayer];
                check(discard(game, game.discardingPlayer, player.hand.front()) == Reason::Ok, "discard applies");
                break;
            }
            case Phase::Infect:
                infectStep(game);
                break;
            case Phase::Over:
                break;
            }
            check(conserved(game), "cubes stay conserved");
            check(game.stationCount() <= kStations, "never more than six stations");
            for (const Player& player : game.players)
                check(player.hand.size() <= kHandLimit || game.phase == Phase::Discard
                      || game.phase == Phase::Draw, "hand limit respected");
        }
        check(game.over(), "the game ended");
        if (game.outcome == Outcome::Won)
            ++won;
        else
            ++lost;
        byOutbreaks += game.outcome == Outcome::LostOutbreaks;
        byCubes += game.outcome == Outcome::LostCubes;
        byCards += game.outcome == Outcome::LostCards;
    }
    std::printf("random games: %d won, %d lost (%d outbreaks, %d cubes, %d cards)\n",
                won, lost, byOutbreaks, byCubes, byCards);
    // all three defeats have to be reachable, otherwise a branch is dead
    check(byOutbreaks > 0 && byCubes > 0, "the board defeats both occur");
}

int main()
{
    testMovement();
    testStationsAndCure();
    testTreatAndEradication();
    testOutbreak();
    testEpidemic();
    testDrawAndInfect();
    testEvents();
    testRoles();
    testVictory();
    testRandomGames();

    std::printf(failures == 0 ? "all rules checks passed\n" : "%d rules checks failed\n", failures);
    return failures == 0 ? 0 : 1;
}
