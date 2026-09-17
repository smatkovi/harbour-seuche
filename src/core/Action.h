// One turn action, or one event card play. Both are serialisable as they stand:
// the network protocol sends these, never state deltas.
#pragma once

#include "Card.h"
#include "Map.h"

#include <cstdint>
#include <vector>

namespace seuche {

enum class ActionKind : std::uint8_t {
    Drive = 0,           // to an adjacent city
    DirectFlight = 1,    // discard the card of the destination
    CharterFlight = 2,   // discard the card of the current city, fly anywhere
    ShuttleFlight = 3,   // station to station
    BuildStation = 4,    // discard the card of the current city (free for Operations)
    Treat = 5,           // remove one cube, or all of a cured colour / for the Medic
    ShareGive = 6,       // hand a card to another player in the same city
    ShareTake = 7,       // take a card from another player in the same city
    DiscoverCure = 8,    // at a station, 5 cards of one colour (4 for the Scientist)
    OperationsFlight = 9,// Operations: from a station to anywhere, discarding any city card
    PlannerTake = 10,    // Planner: pick up an event card from the discard pile
    Pass = 11,           // give up the remaining actions
};

struct Action {
    ActionKind kind = ActionKind::Pass;

    // Which pawn moves. Normally the player at turn; the Dispatcher may name
    // another seat, and then the cards still come from the Dispatcher's hand.
    std::uint8_t pawn = 0xFF;

    City target;                  // destination, or the city a station goes to
    std::uint8_t otherSeat = 0xFF; // Share partner, or the pawn to move onto (Dispatcher)
    PlayerCard card;              // shared card, discarded card, event picked up
    Colour colour = Colour::Blue; // Treat, DiscoverCure
    PlayerCards cure;             // the cards spent on a cure
};

// Event cards cost no action and may be played by anyone, at almost any moment,
// so they travel outside the action stream.
struct EventPlay {
    std::uint8_t seat = 0xFF;
    Event event = Event::QuietNight;
    bool fromRoleCard = false;    // the Planner's stored card

    City city;                    // Grant: where the station goes; Airlift: destination
    std::uint8_t pawn = 0xFF;     // Airlift: whose pawn
    InfectionCards order;         // Forecast: the 6 cards, new top last
    City removed;                 // Resilient: the card taken out of the game
};

} // namespace seuche
