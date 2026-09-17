#include "Reason.h"

namespace seuche {

const char* reasonText(Reason reason)
{
    switch (reason) {
    case Reason::Ok:                  return "in Ordnung";
    case Reason::GameOver:            return "Die Partie ist beendet.";
    case Reason::WrongPhase:          return "In dieser Phase nicht möglich.";
    case Reason::NoActionsLeft:       return "Keine Aktionen mehr übrig.";
    case Reason::NotYourPawn:         return "Nur die Logistikerin bewegt fremde Figuren.";
    case Reason::UnknownCity:         return "Diese Stadt gibt es nicht.";
    case Reason::NotAdjacent:         return "Die Städte sind nicht verbunden.";
    case Reason::SameCity:            return "Die Figur steht bereits dort.";
    case Reason::CardNotInHand:       return "Diese Karte liegt nicht auf der Hand.";
    case Reason::NoStationHere:       return "Hier steht keine Forschungsstation.";
    case Reason::NoStationThere:      return "Dort steht keine Forschungsstation.";
    case Reason::StationAlreadyHere:  return "Hier steht schon eine Forschungsstation.";
    case Reason::NoStationLeft:       return "Alle sechs Stationen stehen bereits; eine muss versetzt werden.";
    case Reason::NothingToTreat:      return "Hier liegt kein Würfel dieser Farbe.";
    case Reason::AlreadyCured:        return "Diese Seuche ist bereits geheilt.";
    case Reason::NotEnoughCards:      return "Zu wenige gleichfarbige Stadtkarten.";
    case Reason::WrongCardColour:     return "Die Karten haben nicht alle dieselbe Farbe.";
    case Reason::NotInSameCity:       return "Beide müssen in derselben Stadt stehen.";
    case Reason::WrongShareCard:      return "Nur die Karte der eigenen Stadt darf getauscht werden.";
    case Reason::NoSuchSeat:          return "Diesen Mitspieler gibt es nicht.";
    case Reason::NoEventInDiscard:    return "Im Ablagestapel liegt keine Ereigniskarte.";
    case Reason::AlreadyStoringEvent: return "Der Krisenplaner bewahrt schon eine Karte auf.";
    case Reason::NotAnEventCard:      return "Das ist keine Ereigniskarte.";
    case Reason::EventNotHeld:        return "Diese Ereigniskarte liegt nicht vor.";
    case Reason::HandLimitFirst:      return "Erst muss auf sieben Handkarten abgeworfen werden.";
    }
    return "?";
}

} // namespace seuche
