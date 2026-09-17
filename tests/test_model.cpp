// Checks the board data and a fresh setup. No test framework: the core is built
// standalone, so a failed check just prints and returns non zero.
#include "../src/core/Map.h"
#include "../src/core/Setup.h"

#include <cstdio>
#include <map>
#include <set>

using namespace seuche;

static int failures = 0;

static void check(bool ok, const std::string& what)
{
    if (!ok) {
        std::printf("FAIL  %s\n", what.c_str());
        ++failures;
    }
}

int main()
{
    std::string problem;
    check(mapIsSane(&problem), "map invariants: " + problem);

    // degree distribution, so a lonely or overloaded city shows up early
    std::map<int, int> degrees;
    int edges = 0;
    for (int id = 0; id < kCities; ++id) {
        const int degree = static_cast<int>(neighbours(City(id)).size());
        ++degrees[degree];
        edges += degree;
        check(degree >= 1 && degree <= 6, std::string(cityInfo(City(id)).name) + " has degree " + std::to_string(degree));
    }
    std::printf("cities %d, edges %d, average degree %.2f\n", kCities, edges / 2, double(edges) / kCities);
    for (const auto& entry : degrees)
        std::printf("  degree %d: %d cities\n", entry.first, entry.second);

    // spot checks against the board this port copies: the shape of the graph is
    // the point, so a typo in the edge list has to fail here
    check(edges / 2 == 93, "93 connections");
    check(kStartCity == cityByKey("atlanta"), "the game starts in Atlanta");
    const struct { const char* city; const char* neighbours; } wanted[] = {
        {"atlanta",      "chicago,washington,miami"},
        {"santiago",     "lima"},
        {"osaka",        "tokio,taipeh"},
        {"peking",       "schanghai,seoul"},
        {"istanbul",     "mailand,stpetersburg,algier,moskau,kairo,bagdad"},
        {"hongkong",     "schanghai,taipeh,kalkutta,bangkok,hochiminh,manila"},
        {"sanfrancisco", "chicago,losangeles,tokio,manila"},
        {"kalkutta",     "delhi,chennai,hongkong,bangkok"},
        {"khartum",      "kairo,lagos,kinshasa,johannesburg"},
        {"sydney",       "losangeles,manila,jakarta"},
    };
    for (const auto& entry : wanted) {
        const City city = cityByKey(entry.city);
        check(city.valid(), std::string("unknown key ") + entry.city);
        std::set<std::string> want, have;
        std::string token;
        for (const char* p = entry.neighbours; ; ++p) {
            if (*p == ',' || *p == '\0') {
                want.insert(token);
                token.clear();
                if (*p == '\0')
                    break;
            } else {
                token += *p;
            }
        }
        for (City next : neighbours(city))
            have.insert(cityInfo(next).key);
        check(want == have, std::string(entry.city) + " has the wrong neighbours");
    }

    // unique names and keys
    std::set<std::string> names, keys;
    for (int id = 0; id < kCities; ++id) {
        names.insert(cityInfo(City(id)).name);
        keys.insert(cityInfo(City(id)).key);
        check(cityByKey(cityInfo(City(id)).key) == City(id), "key round trip");
    }
    check(names.size() == kCities, "city names are unique");
    check(keys.size() == kCities, "city keys are unique");

    for (int colour = 0; colour < kBaseColours; ++colour) {
        int count = 0;
        for (int id = 0; id < kCities; ++id)
            count += colourOf(id) == Colour(colour) ? 1 : 0;
        check(count == kCitiesPerColour, "twelve cities in colour " + std::to_string(colour));
    }

    // setup
    for (int players = 2; players <= 4; ++players) {
        std::mt19937 rng(1234 + players);
        SetupOptions options;
        options.players = players;
        const Game game = newGame(options, rng);

        check(static_cast<int>(game.players.size()) == players, "seat count");
        std::set<Role> roles;
        for (const Player& player : game.players) {
            roles.insert(player.role);
            check(static_cast<int>(player.hand.size()) == startingHand(players), "hand size");
            check(player.city == kStartCity, "pawns start in Chicago");
        }
        check(static_cast<int>(roles.size()) == players, "roles are all different");

        check(game.stationCount() == 1, "one station at the start");
        check(game.hasStation(kStartCity), "the station is in Chicago");

        const int dealt = players * startingHand(players);
        const int expected = kCities + kBaseEvents - dealt + epidemicCards(options.difficulty);
        check(static_cast<int>(game.playerDeck.size()) == expected, "player deck size");

        int epidemics = 0;
        for (PlayerCard card : game.playerDeck)
            epidemics += card.isEpidemic() ? 1 : 0;
        check(epidemics == epidemicCards(options.difficulty), "epidemic count");

        check(game.infectionDeck.size() == kCities - 9, "infection deck size");
        check(game.infectionDiscard.size() == 9, "nine cities infected");

        int placed = 0;
        for (int colour = 0; colour < kBaseColours; ++colour) {
            placed += game.totalCubes(Colour(colour));
            check(game.supply[colour] + game.totalCubes(Colour(colour)) == kCubesPerColour,
                  "cube bookkeeping for colour " + std::to_string(colour));
        }
        check(placed == 18, "eighteen cubes on the board");

        // one epidemic per deck section, never two in the same section
        const int sections = epidemicCards(options.difficulty);
        const int size = static_cast<int>(game.playerDeck.size());
        std::vector<int> perSection(sections, 0);
        int index = 0, taken = 0;
        for (int section = 0; section < sections; ++section) {
            const int length = (size - taken) / (sections - section);
            for (int card = 0; card < length; ++card, ++index)
                perSection[section] += game.playerDeck[index].isEpidemic() ? 1 : 0;
            taken += length;
        }
        for (int section = 0; section < sections; ++section)
            check(perSection[section] == 1, "one epidemic in section " + std::to_string(section));
    }

    std::printf(failures == 0 ? "all checks passed\n" : "%d checks failed\n", failures);
    return failures == 0 ? 0 : 1;
}
