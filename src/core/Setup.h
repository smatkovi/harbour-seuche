// Building a fresh game: decks, roles, the nine starting infections and the
// epidemic cards shuffled one per deck section.
#pragma once

#include "State.h"

#include <random>

namespace seuche {

struct SetupOptions {
    int players = 4;                             // 2..4, up to 5 with a module on
    Difficulty difficulty = Difficulty::Normal;
    std::vector<Role> roles;                     // empty = draw at random, all different
    Modules modules;                             // stored, not yet acted on
};

// Deals a legal starting position. The rng is the only source of randomness, so
// the same seed gives the same game on every platform. Modules are carried into
// the Game as they stand — only the cube supply and the seat count already look
// at them, the rest waits for the engine.
Game newGame(const SetupOptions& options, std::mt19937& rng);

// The two deck builders, exposed for the tests.
PlayerCards fullPlayerDeck();                    // 48 city + 5 event cards, unshuffled
InfectionCards fullInfectionDeck();              // all 48 cities, unshuffled

// Splits `deck` into `epidemics` near equal piles, shuffles one epidemic card
// into each, and stacks them again. Top of the deck is back().
void mixInEpidemics(PlayerCards& deck, int epidemics, std::mt19937& rng);

} // namespace seuche
