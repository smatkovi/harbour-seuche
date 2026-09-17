// The whole game state. Everything here except the order of the two decks is
// open information, so the network layer mirrors this struct as it stands and
// keeps only the shuffles on the host.
#pragma once

#include "Card.h"
#include "Map.h"
#include "Role.h"

#include <array>
#include <cstdint>
#include <vector>

namespace seuche {

constexpr int kCubesPerColour = 24;
constexpr int kMutationCubes = 12;       // the purple disease brings fewer
constexpr int kMaxPlayers = 5;           // 5 needs the first expansion
constexpr int kStations = 6;
constexpr int kHandLimit = 7;
constexpr int kCubesPerCity = 3;         // the fourth cube is an outbreak
constexpr int kLosingOutbreaks = 8;
constexpr int kActionsPerTurn = 4;
constexpr int kDrawsPerTurn = 2;
constexpr int kForecastCards = 6;

constexpr std::array<int, 7> kInfectionRates = {2, 2, 2, 3, 3, 4, 4};

constexpr int cubesInBox(Colour colour)
{
    return colour == Colour::Purple ? kMutationCubes : kCubesPerColour;
}

// Legendary (7 epidemics) comes with the first expansion.
enum class Difficulty : std::uint8_t { Introduction = 0, Normal = 1, Heroic = 2, Legendary = 3 };
constexpr int epidemicCards(Difficulty difficulty) { return 4 + static_cast<int>(difficulty); }

constexpr int startingHand(int players) { return players == 2 ? 4 : (players == 3 ? 3 : 2); }

// Which modules are switched on. Nothing here is implemented yet; the flags
// exist so the state, the card ids and the save format already have room and a
// module never forces a renumbering.
//
// Note on `bioTerrorist`: it is the one module with hidden information — one
// player moves secretly against the team. Every other module keeps the open
// state the network layer relies on, so that one needs a separate protocol path
// and cannot simply mirror `Game`.
struct Modules {
    // first expansion
    bool virulentStrain = false;   // epidemic cards with individual effects
    bool mutation = false;         // the purple fifth disease
    bool bioTerrorist = false;     // one player against the team, hidden moves
    bool quarantines = false;      // quarantine markers
    // second expansion
    bool lab = false;              // curing becomes a multi step lab procedure
    bool worldwidePanic = false;   // cities panic and fall, pawns get restricted
    bool teamPlay = false;         // two teams of two
    // third expansion
    bool emergencyEvents = false;
    bool superbug = false;
    bool hinterlands = false;      // adds cities behind id 47

    bool any() const
    {
        return virulentStrain || mutation || bioTerrorist || quarantines || lab
            || worldwidePanic || teamPlay || emergencyEvents || superbug || hinterlands;
    }
};

enum class CureState : std::uint8_t { None = 0, Cured = 1, Eradicated = 2 };

enum class Phase : std::uint8_t {
    Actions = 0,         // the player at turn has actions left
    Draw = 1,            // two player cards are being drawn
    Discard = 2,         // someone is over the hand limit and must discard
    Infect = 3,          // infection cards are being turned
    Over = 4,
};

enum class Outcome : std::uint8_t {
    Running = 0,
    Won = 1,             // all four cures discovered
    LostOutbreaks = 2,   // the outbreak marker reached 8
    LostCubes = 3,       // a cube supply ran out
    LostCards = 4,       // the player deck ran out
};

struct Player {
    Role role = Role::Medic;
    City city = kStartCity;
    PlayerCards hand;
    PlayerCard storedEvent;           // Planner only, outside the hand limit
    bool operationsFlightUsed = false; // Operations, reset every turn
};

struct Game {
    // board
    std::array<std::array<std::uint8_t, kColourSlots>, kCities> cubes = {};
    CitySet stations;
    std::array<std::uint8_t, kColourSlots> supply = {};   // cubes still in the box
    std::array<CureState, kColourSlots> cures = {};
    std::uint8_t colours = kBaseColours;                  // 5 with the mutation module

    // players
    std::vector<Player> players;
    std::uint8_t atTurn = 0;
    std::uint8_t actionsLeft = kActionsPerTurn;
    std::uint8_t drawsLeft = 0;
    std::uint8_t discardingPlayer = 0xFF;   // valid only in Phase::Discard

    // decks; back() is the top of the deck, so drawing is pop_back()
    PlayerCards playerDeck;
    PlayerCards playerDiscard;
    InfectionCards infectionDeck;
    InfectionCards infectionDiscard;
    InfectionCards removedFromGame;        // taken out by "Zähe Bevölkerung"

    // tracks
    std::uint8_t outbreaks = 0;
    std::uint8_t infectionRateIndex = 0;
    std::uint8_t infectionsLeft = 0;       // cards still to turn in Phase::Infect
    bool quietNight = false;               // set by the event, consumed by the next infect phase

    Difficulty difficulty = Difficulty::Normal;
    Modules modules;
    Phase phase = Phase::Actions;
    Outcome outcome = Outcome::Running;

    // queries
    int infectionRate() const { return kInfectionRates[infectionRateIndex]; }
    int colourCount() const { return colours; }
    int cubes_(City city, Colour colour) const { return cubes[city.id][static_cast<int>(colour)]; }
    int totalCubes(Colour colour) const;
    bool hasStation(City city) const { return stations.test(city.id); }
    int stationCount() const { return static_cast<int>(stations.count()); }
    const Player& current() const { return players[atTurn]; }
    Player& current() { return players[atTurn]; }
    bool over() const { return outcome != Outcome::Running; }

    // Where a role sits right now, 0xFF when nobody plays it.
    int seatOf(Role role) const;
    // True while the Quarantine Specialist shields this city.
    bool shielded(City city) const;
};

} // namespace seuche
