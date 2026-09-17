// Player and infection cards. The infection deck holds nothing but city ids, so
// only the player deck needs its own card type.
#pragma once

#include "Map.h"

#include <cstdint>
#include <vector>

namespace seuche {

enum class Event : std::uint8_t {
    QuietNight = 0,      // the next infection step is skipped
    Forecast = 1,        // look at the top 6 infection cards, reorder them
    Grant = 2,           // free research station anywhere
    Airlift = 3,         // move any pawn anywhere
    Resilient = 4,       // remove one card from the infection discard pile
    // 5.. are reserved for the expansions' event cards
};

constexpr int kBaseEvents = 5;

// Epidemic cards are identical in the base game. The first expansion's virulent
// strain gives each one its own effect, so an epidemic card carries a kind.
enum class Epidemic : std::uint8_t { Plain = 0 };

constexpr int kBaseEpidemics = 1;

// Player card id — three fixed ranges, sized so that no expansion has to
// renumber what already exists:
//   0..47    city card, same numbering as City (expansion cities append at 48,
//            shifting the ranges below only in a future, versioned deck format)
//   64..95   event card, 64 + Event
//   96..111  epidemic card, 96 + Epidemic
constexpr int kEventBase = 64;
constexpr int kEventSlots = 32;
constexpr int kEpidemicBase = 96;
constexpr int kEpidemicSlots = 16;
constexpr int kCardIds = kEpidemicBase + kEpidemicSlots;
struct PlayerCard {
    std::uint8_t id = 0xFF;

    constexpr PlayerCard() = default;
    constexpr explicit PlayerCard(std::uint8_t value) : id(value) {}

    constexpr bool valid() const { return isCity() || isEvent() || isEpidemic(); }
    constexpr bool isCity() const { return id < kCities; }
    constexpr bool isEvent() const { return id >= kEventBase && id < kEventBase + kEventSlots; }
    constexpr bool isEpidemic() const { return id >= kEpidemicBase && id < kCardIds; }

    constexpr City city() const { return isCity() ? City(id) : City(); }
    constexpr Event event() const { return Event(id - kEventBase); }
    constexpr Epidemic epidemic() const { return Epidemic(id - kEpidemicBase); }
    constexpr Colour colour() const { return colourOf(id); }   // cities only

    constexpr bool operator==(const PlayerCard& other) const { return id == other.id; }
    constexpr bool operator!=(const PlayerCard& other) const { return id != other.id; }
};

constexpr PlayerCard cityCard(City city) { return PlayerCard(city.id); }
constexpr PlayerCard eventCard(Event event)
{
    return PlayerCard(static_cast<std::uint8_t>(kEventBase + static_cast<int>(event)));
}
constexpr PlayerCard epidemicCard(Epidemic kind = Epidemic::Plain)
{
    return PlayerCard(static_cast<std::uint8_t>(kEpidemicBase + static_cast<int>(kind)));
}

constexpr PlayerCard kEpidemicCard = epidemicCard();

using PlayerCards = std::vector<PlayerCard>;
using InfectionCards = std::vector<City>;

const char* eventName(Event event);
const char* eventText(Event event);

// A hand is a list, not a set: two epidemic cards never coexist, but the same
// card may sit in a hand and on the Contingency Planner's role card at once.
int countColour(const PlayerCards& hand, Colour colour);
bool hasCard(const PlayerCards& hand, PlayerCard card);
bool removeCard(PlayerCards& hand, PlayerCard card);   // removes one copy

} // namespace seuche
