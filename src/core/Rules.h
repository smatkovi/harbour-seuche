// The engine interface. Every state change in the game goes through one of
// these; nothing else touches Game's fields. Implementation is the next step —
// this header fixes the shape the UI, the AI and the network layer build on.
#pragma once

#include "Action.h"
#include "Reason.h"
#include "State.h"

#include <random>
#include <vector>

namespace seuche {

// --- actions ---------------------------------------------------------------

Reason check(const Game& game, const Action& action);
std::vector<Action> legalActions(const Game& game);

// Applies a legal action. Returns Reason::Ok and mutates, or refuses and leaves
// the game untouched. Drawing and infecting happen in the phase steps below, so
// an action never draws a card as a side effect.
Reason apply(Game& game, const Action& action, std::mt19937& rng);

// --- events ----------------------------------------------------------------

Reason check(const Game& game, const EventPlay& play);
Reason apply(Game& game, const EventPlay& play, std::mt19937& rng);

// --- phase steps -----------------------------------------------------------

// Ends the action phase; from there the caller drives draw and infect until the
// turn passes on. Each step does exactly one card, so the UI can animate and
// events can be played in between.
void endActions(Game& game);
void drawPlayerCard(Game& game, std::mt19937& rng);   // handles epidemics
Reason discard(Game& game, int seat, PlayerCard card);
void infectStep(Game& game);                          // one infection card
void endTurn(Game& game);

// --- board changes the steps above are built from --------------------------

// Places one cube and resolves the outbreak chain. Respects the Quarantine
// Specialist, the Medic on cured colours, and eradication. Sets Outcome when a
// supply runs dry or the eighth outbreak happens.
void placeCube(Game& game, City city, Colour colour);
void removeCubes(Game& game, City city, Colour colour, int count);
void checkEradication(Game& game, Colour colour);

} // namespace seuche
