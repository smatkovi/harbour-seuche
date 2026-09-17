// Why an action was refused. The UI turns these into sentences, the AI uses them
// to prune, and the tests assert on them.
#pragma once

#include <cstdint>

namespace seuche {

enum class Reason : std::uint8_t {
    Ok = 0,
    GameOver,
    WrongPhase,
    NoActionsLeft,
    NotYourPawn,          // only the Dispatcher moves someone else
    UnknownCity,
    NotAdjacent,
    SameCity,
    CardNotInHand,
    NoStationHere,
    NoStationThere,
    StationAlreadyHere,
    NoStationLeft,        // all six are placed and none was named for moving
    NothingToTreat,
    AlreadyCured,
    NotEnoughCards,       // cure: fewer than 5 (4 for the Scientist)
    WrongCardColour,
    NotInSameCity,        // share knowledge
    WrongShareCard,       // not the card of the current city, and not the Researcher
    NoSuchSeat,
    NoEventInDiscard,     // Planner
    AlreadyStoringEvent,  // Planner
    NotAnEventCard,
    EventNotHeld,
    HandLimitFirst,       // someone must discard before play continues
};

const char* reasonText(Reason reason);

} // namespace seuche
