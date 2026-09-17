// The 48-city board. City ids are stable: they are the card numbering, the save
// format and the network protocol all at once, so they must never be reordered.
#pragma once

#include <array>
#include <bitset>
#include <cstdint>
#include <string>
#include <vector>

namespace seuche {

// Purple is the mutation disease of the first expansion: no cities of its own,
// its cubes only ever arrive through mutation cards. It is always the last slot
// so the base game can simply stop counting at kBaseColours.
enum class Colour : std::uint8_t { Blue = 0, Yellow = 1, Black = 2, Red = 3, Purple = 4 };

constexpr int kBaseColours = 4;
constexpr int kColourSlots = 5;   // array width, mutation included
constexpr int kCities = 48;
constexpr int kCitiesPerColour = 12;

// City id 0..47, grouped by colour: 0..11 blue, 12..23 yellow, 24..35 black,
// 36..47 red. The grouping is what lets "five cards of one colour" be a range
// check instead of a lookup. Expansions that add cities append behind 47, so
// nothing already numbered ever moves.
struct City {
    std::uint8_t id = 0xFF;

    constexpr City() = default;
    constexpr explicit City(std::uint8_t value) : id(value) {}

    constexpr bool valid() const { return id < kCities; }
    constexpr Colour colour() const { return Colour(id / kCitiesPerColour); }

    constexpr bool operator==(const City& other) const { return id == other.id; }
    constexpr bool operator!=(const City& other) const { return id != other.id; }
};

constexpr Colour colourOf(int cityId) { return Colour(cityId / kCitiesPerColour); }
constexpr int firstCityOf(Colour colour) { return kCitiesPerColour * static_cast<int>(colour); }

using CitySet = std::bitset<kCities>;

struct CityInfo {
    const char* name;        // display name, German
    const char* key;         // stable ascii key for assets and save files
    float latitude;
    float longitude;
    std::uint32_t people;    // metropolitan population, decides the starting seat
};

// The seat of the disease control institute: first research station, all pawns
// start here.
constexpr City kStartCity{2};   // Atlanta

const CityInfo& cityInfo(City city);
City cityByKey(const std::string& key);          // invalid City when unknown

// Undirected; every edge appears in both cities' lists.
const std::vector<City>& neighbours(City city);
bool adjacent(City a, City b);

// Checks the invariants the rest of the code relies on: 12 cities per colour,
// symmetric edges, no self loops or duplicates, one connected component, and
// every colour region internally connected.
bool mapIsSane(std::string* problem = nullptr);

} // namespace seuche
