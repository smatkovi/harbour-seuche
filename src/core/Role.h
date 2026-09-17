// The seven roles. Every ability is a rule exception, so the engine asks the
// role rather than special casing player indices.
#pragma once

#include <cstdint>

namespace seuche {

// Ids 0..6 are the base game. The expansions' roles append from 7 upwards, so a
// saved game keeps its meaning when a module is switched on later.
enum class Role : std::uint8_t {
    Medic = 0,           // treat removes every cube; cured cubes go on entry, and stay away
    Researcher = 1,      // may hand over any city card, not only the current one
    Scientist = 2,       // needs 4 cards instead of 5 for a cure
    Dispatcher = 3,      // may move other pawns, and move a pawn onto another pawn
    Operations = 4,      // free station; once per turn station -> anywhere for any card
    Quarantine = 5,      // no cubes and no outbreaks in her city and its neighbours
    Planner = 6,         // may keep one event card from the discard pile
};

constexpr int kBaseRoles = 7;
constexpr int kRoleSlots = 32;

constexpr int kCardsForCure = 5;
constexpr int cardsForCure(Role role) { return role == Role::Scientist ? 4 : kCardsForCure; }

const char* roleName(Role role);
const char* roleAbility(Role role);

} // namespace seuche
