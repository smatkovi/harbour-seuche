#include "Map.h"

#include <algorithm>
#include <queue>

namespace seuche {
namespace {

// Ids are positional: index 0..11 blue, 12..23 yellow, 24..35 black, 36..47 red.
// Populations are our own metropolitan estimates and only decide who starts.
const std::array<CityInfo, kCities> kCities_ = {{
    // blue — North America and Europe
    {"San Francisco",     "sanfrancisco",   37.8f, -122.4f,   5'900'000},
    {"Chicago",           "chicago",        41.9f,  -87.6f,   9'500'000},
    {"Atlanta",           "atlanta",        33.7f,  -84.4f,   6'100'000},
    {"Montreal",          "montreal",       45.5f,  -73.6f,   4'300'000},
    {"New York",          "newyork",        40.7f,  -74.0f,  19'000'000},
    {"Washington",        "washington",     38.9f,  -77.0f,   6'300'000},
    {"London",            "london",         51.5f,   -0.1f,   9'500'000},
    {"Madrid",            "madrid",         40.4f,   -3.7f,   6'700'000},
    {"Paris",             "paris",          48.9f,    2.4f,  11'000'000},
    {"Essen",             "essen",          51.5f,    7.0f,   5'100'000},
    {"Mailand",           "mailand",        45.5f,    9.2f,   4'300'000},
    {"Sankt Petersburg",  "stpetersburg",   59.9f,   30.3f,   5'400'000},
    // yellow — Latin America and Africa
    {"Los Angeles",       "losangeles",     34.1f, -118.2f,  13'200'000},
    {"Mexiko-Stadt",      "mexikostadt",    19.4f,  -99.1f,  21'800'000},
    {"Miami",             "miami",          25.8f,  -80.2f,   6'200'000},
    {"Bogotá",            "bogota",          4.7f,  -74.1f,  11'000'000},
    {"Lima",              "lima",          -12.0f,  -77.0f,  10'700'000},
    {"Santiago",          "santiago",      -33.5f,  -70.7f,   7'100'000},
    {"São Paulo",         "saopaulo",      -23.5f,  -46.6f,  22'400'000},
    {"Buenos Aires",      "buenosaires",   -34.6f,  -58.4f,  15'400'000},
    {"Lagos",             "lagos",           6.5f,    3.4f,  15'400'000},
    {"Kinshasa",          "kinshasa",       -4.3f,   15.3f,  17'100'000},
    {"Khartum",           "khartum",        15.5f,   32.5f,   6'300'000},
    {"Johannesburg",      "johannesburg",  -26.2f,   28.0f,   6'100'000},
    // black — North Africa, Eastern Europe, Middle East, South Asia
    {"Algier",            "algier",         36.8f,    3.1f,   3'400'000},
    {"Istanbul",          "istanbul",       41.0f,   29.0f,  15'800'000},
    {"Moskau",            "moskau",         55.8f,   37.6f,  12'600'000},
    {"Kairo",             "kairo",          30.0f,   31.2f,  22'200'000},
    {"Bagdad",            "bagdad",         33.3f,   44.4f,   7'700'000},
    {"Teheran",           "teheran",        35.7f,   51.4f,   9'600'000},
    {"Riad",              "riad",           24.7f,   46.7f,   7'700'000},
    {"Karatschi",         "karatschi",      24.9f,   67.0f,  16'800'000},
    {"Delhi",             "delhi",          28.6f,   77.2f,  32'900'000},
    {"Mumbai",            "mumbai",         19.1f,   72.9f,  21'300'000},
    {"Chennai",           "chennai",        13.1f,   80.3f,  11'500'000},
    {"Kalkutta",          "kalkutta",       22.6f,   88.4f,  15'100'000},
    // red — East and South East Asia, Oceania
    {"Peking",            "peking",         39.9f,  116.4f,  21'500'000},
    {"Seoul",             "seoul",          37.6f,  127.0f,  25'600'000},
    {"Tokio",             "tokio",          35.7f,  139.7f,  37'400'000},
    {"Schanghai",         "schanghai",      31.2f,  121.5f,  27'100'000},
    {"Hongkong",          "hongkong",       22.3f,  114.2f,   7'500'000},
    {"Taipeh",            "taipeh",         25.0f,  121.6f,   7'000'000},
    {"Osaka",             "osaka",          34.7f,  135.5f,  19'100'000},
    {"Bangkok",           "bangkok",        13.8f,  100.5f,  10'500'000},
    {"Ho-Chi-Minh-Stadt", "hochiminh",      10.8f,  106.7f,   9'300'000},
    {"Manila",            "manila",         14.6f,  121.0f,  13'900'000},
    {"Jakarta",           "jakarta",        -6.2f,  106.8f,  10'600'000},
    {"Sydney",            "sydney",        -33.9f,  151.2f,   5'300'000},
}};

// Undirected edges, listed once. Degrees run from 1 (Santiago) to 6 (Istanbul,
// Hongkong); every colour region is reachable within itself without leaving it.
constexpr std::uint8_t kEdges[][2] = {
    // blue
    { 0,  1}, { 1,  2}, { 1,  3}, { 2,  5}, { 3,  4}, { 3,  5}, { 4,  5}, { 4,  6},
    { 4,  7}, { 6,  7}, { 6,  8}, { 6,  9}, { 7,  8}, { 8,  9}, { 8, 10}, { 9, 10},
    { 9, 11},
    // yellow
    {12, 13}, {13, 14}, {13, 15}, {13, 16}, {14, 15}, {15, 16}, {15, 18}, {15, 19},
    {16, 17}, {18, 19}, {18, 20}, {20, 21}, {20, 22}, {21, 22}, {21, 23}, {22, 23},
    // black
    {24, 25}, {24, 27}, {25, 26}, {25, 27}, {25, 28}, {26, 29}, {27, 28}, {27, 30},
    {28, 29}, {28, 30}, {28, 31}, {29, 31}, {29, 32}, {30, 31}, {31, 32}, {31, 33},
    {32, 33}, {32, 34}, {32, 35}, {33, 34}, {34, 35},
    // red
    {36, 37}, {36, 39}, {37, 38}, {37, 39}, {38, 39}, {38, 42}, {39, 40}, {39, 41},
    {40, 41}, {40, 43}, {40, 44}, {40, 45}, {41, 42}, {41, 45}, {43, 44}, {43, 46},
    {44, 45}, {44, 46}, {45, 47}, {46, 47},
    // between the regions
    { 0, 12}, { 0, 38}, { 0, 45}, { 1, 12}, { 1, 13}, { 2, 14}, { 5, 14}, { 7, 18},
    { 7, 24}, { 8, 24}, {10, 25}, {11, 25}, {11, 26}, {12, 47}, {22, 27}, {34, 43},
    {34, 46}, {35, 40}, {35, 43},
};

std::array<std::vector<City>, kCities> buildAdjacency()
{
    std::array<std::vector<City>, kCities> adjacency;
    for (const auto& edge : kEdges) {
        adjacency[edge[0]].push_back(City(edge[1]));
        adjacency[edge[1]].push_back(City(edge[0]));
    }
    for (auto& list : adjacency)
        std::sort(list.begin(), list.end(), [](City a, City b) { return a.id < b.id; });
    return adjacency;
}

const std::array<std::vector<City>, kCities>& adjacency()
{
    static const std::array<std::vector<City>, kCities> table = buildAdjacency();
    return table;
}

const std::vector<City> kNoNeighbours;

// Breadth first over the cities allowed by `allowed`, starting at `start`.
CitySet reachable(City start, const CitySet& allowed)
{
    CitySet seen;
    if (!start.valid() || !allowed.test(start.id))
        return seen;

    std::queue<City> pending;
    pending.push(start);
    seen.set(start.id);
    while (!pending.empty()) {
        const City city = pending.front();
        pending.pop();
        for (City next : adjacency()[city.id]) {
            if (allowed.test(next.id) && !seen.test(next.id)) {
                seen.set(next.id);
                pending.push(next);
            }
        }
    }
    return seen;
}

} // namespace

const CityInfo& cityInfo(City city)
{
    static const CityInfo unknown = {"?", "?", 0.0f, 0.0f, 0};
    return city.valid() ? kCities_[city.id] : unknown;
}

City cityByKey(const std::string& key)
{
    for (int id = 0; id < kCities; ++id) {
        if (key == kCities_[id].key)
            return City(static_cast<std::uint8_t>(id));
    }
    return City();
}

const std::vector<City>& neighbours(City city)
{
    return city.valid() ? adjacency()[city.id] : kNoNeighbours;
}

bool adjacent(City a, City b)
{
    if (!a.valid() || !b.valid())
        return false;
    const auto& list = adjacency()[a.id];
    return std::find(list.begin(), list.end(), b) != list.end();
}

bool mapIsSane(std::string* problem)
{
    const auto fail = [problem](const std::string& text) {
        if (problem)
            *problem = text;
        return false;
    };

    for (const auto& edge : kEdges) {
        if (edge[0] == edge[1])
            return fail("self loop at city " + std::to_string(edge[0]));
        if (edge[0] >= kCities || edge[1] >= kCities)
            return fail("edge leaves the board");
    }

    for (int id = 0; id < kCities; ++id) {
        const auto& list = adjacency()[id];
        if (list.empty())
            return fail(std::string(kCities_[id].name) + " has no neighbours");
        if (std::adjacent_find(list.begin(), list.end()) != list.end())
            return fail(std::string(kCities_[id].name) + " has a duplicate edge");
        for (City next : list) {
            if (!adjacent(next, City(static_cast<std::uint8_t>(id))))
                return fail(std::string(kCities_[id].name) + " has a one way edge");
        }
    }

    CitySet everything;
    everything.set();
    if (reachable(City(0), everything).count() != kCities)
        return fail("the board is not one connected component");

    for (int colour = 0; colour < kBaseColours; ++colour) {
        CitySet region;
        const int first = firstCityOf(Colour(colour));
        for (int offset = 0; offset < kCitiesPerColour; ++offset)
            region.set(first + offset);
        if (reachable(City(static_cast<std::uint8_t>(first)), region).count() != kCitiesPerColour)
            return fail("colour region " + std::to_string(colour) + " is not connected within itself");
    }

    if (!kStartCity.valid())
        return fail("start city is not on the board");
    return true;
}

} // namespace seuche
