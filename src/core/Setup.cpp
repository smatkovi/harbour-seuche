#include "Setup.h"

#include <algorithm>
#include <numeric>

namespace seuche {
namespace {

// The nine opening infections: three cities with three cubes, three with two,
// three with one.
constexpr int kOpeningCubes[] = {3, 3, 3, 2, 2, 2, 1, 1, 1};

} // namespace

PlayerCards fullPlayerDeck()
{
    PlayerCards deck;
    deck.reserve(kCities + kBaseEvents);
    for (int id = 0; id < kCities; ++id)
        deck.push_back(cityCard(City(static_cast<std::uint8_t>(id))));
    for (int event = 0; event < kBaseEvents; ++event)
        deck.push_back(eventCard(Event(event)));
    return deck;
}

InfectionCards fullInfectionDeck()
{
    InfectionCards deck;
    deck.reserve(kCities);
    for (int id = 0; id < kCities; ++id)
        deck.push_back(City(static_cast<std::uint8_t>(id)));
    return deck;
}

void mixInEpidemics(PlayerCards& deck, int epidemics, std::mt19937& rng)
{
    if (epidemics <= 0)
        return;

    // Bottom pile first: the deck is stored with the top at back(), so the piles
    // are stacked in the order they are built.
    PlayerCards stacked;
    stacked.reserve(deck.size() + epidemics);

    const int total = static_cast<int>(deck.size());
    int taken = 0;
    for (int pile = 0; pile < epidemics; ++pile) {
        const int size = (total - taken) / (epidemics - pile);
        PlayerCards part(deck.begin() + taken, deck.begin() + taken + size);
        taken += size;
        part.push_back(kEpidemicCard);
        std::shuffle(part.begin(), part.end(), rng);
        stacked.insert(stacked.end(), part.begin(), part.end());
    }
    deck.swap(stacked);
}

Game newGame(const SetupOptions& options, std::mt19937& rng)
{
    Game game;
    const int seats = std::clamp(options.players, 2, options.modules.any() ? kMaxPlayers : 4);
    game.difficulty = options.difficulty;
    game.modules = options.modules;
    game.colours = static_cast<std::uint8_t>(options.modules.mutation ? kColourSlots : kBaseColours);

    game.supply.fill(0);
    for (int colour = 0; colour < game.colours; ++colour)
        game.supply[colour] = static_cast<std::uint8_t>(cubesInBox(Colour(colour)));
    game.cures.fill(CureState::None);
    game.stations.set(kStartCity.id);

    // roles
    std::vector<Role> roles = options.roles;
    if (static_cast<int>(roles.size()) < seats) {
        std::vector<Role> pool;
        for (int role = 0; role < kBaseRoles; ++role) {
            const Role candidate = Role(role);
            if (std::find(roles.begin(), roles.end(), candidate) == roles.end())
                pool.push_back(candidate);
        }
        std::shuffle(pool.begin(), pool.end(), rng);
        roles.insert(roles.end(), pool.begin(), pool.begin() + (seats - roles.size()));
    }
    roles.resize(seats);

    game.players.resize(seats);
    for (int seat = 0; seat < seats; ++seat) {
        game.players[seat].role = roles[seat];
        game.players[seat].city = kStartCity;
    }

    // opening infections, before the players draw
    game.infectionDeck = fullInfectionDeck();
    std::shuffle(game.infectionDeck.begin(), game.infectionDeck.end(), rng);
    for (int cubes : kOpeningCubes) {
        const City city = game.infectionDeck.back();
        game.infectionDeck.pop_back();
        const int colour = static_cast<int>(city.colour());
        game.cubes[city.id][colour] = static_cast<std::uint8_t>(cubes);
        game.supply[colour] -= static_cast<std::uint8_t>(cubes);
        game.infectionDiscard.push_back(city);
    }

    // hands off the shuffled deck, epidemics only afterwards
    PlayerCards deck = fullPlayerDeck();
    std::shuffle(deck.begin(), deck.end(), rng);
    const int handSize = startingHand(seats);
    for (int seat = 0; seat < seats; ++seat) {
        for (int card = 0; card < handSize; ++card) {
            game.players[seat].hand.push_back(deck.back());
            deck.pop_back();
        }
    }

    mixInEpidemics(deck, epidemicCards(options.difficulty), rng);
    game.playerDeck.swap(deck);

    // the largest city on a hand starts; ties go to the lower seat
    std::uint32_t best = 0;
    for (int seat = 0; seat < seats; ++seat) {
        for (PlayerCard card : game.players[seat].hand) {
            if (!card.isCity())
                continue;
            const std::uint32_t people = cityInfo(card.city()).people;
            if (people > best) {
                best = people;
                game.atTurn = static_cast<std::uint8_t>(seat);
            }
        }
    }

    game.phase = Phase::Actions;
    game.actionsLeft = kActionsPerTurn;
    return game;
}

} // namespace seuche
