#include "Card.h"

#include <algorithm>

namespace seuche {

const char* eventName(Event event)
{
    switch (event) {
    case Event::QuietNight: return "Ruhige Nacht";
    case Event::Forecast:   return "Prognose";
    case Event::Grant:      return "Sonderbudget";
    case Event::Airlift:    return "Lufttransport";
    case Event::Resilient:  return "Zähe Bevölkerung";
    }
    return "?";
}

const char* eventText(Event event)
{
    switch (event) {
    case Event::QuietNight: return "Die nächste Infektionsphase entfällt.";
    case Event::Forecast:   return "Die obersten 6 Infektionskarten ansehen und in beliebiger Reihenfolge zurücklegen.";
    case Event::Grant:      return "Eine Forschungsstation in eine beliebige Stadt setzen, ohne Karte abzuwerfen.";
    case Event::Airlift:    return "Eine beliebige Figur in eine beliebige Stadt setzen.";
    case Event::Resilient:  return "Eine Karte aus dem Infektionsablagestapel aus dem Spiel nehmen.";
    }
    return "?";
}

int countColour(const PlayerCards& hand, Colour colour)
{
    return static_cast<int>(std::count_if(hand.begin(), hand.end(), [colour](PlayerCard card) {
        return card.isCity() && card.colour() == colour;
    }));
}

bool hasCard(const PlayerCards& hand, PlayerCard card)
{
    return std::find(hand.begin(), hand.end(), card) != hand.end();
}

bool removeCard(PlayerCards& hand, PlayerCard card)
{
    const auto it = std::find(hand.begin(), hand.end(), card);
    if (it == hand.end())
        return false;
    hand.erase(it);
    return true;
}

} // namespace seuche
