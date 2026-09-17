#include "Role.h"

namespace seuche {

const char* roleName(Role role)
{
    switch (role) {
    case Role::Medic:      return "Ärztin";
    case Role::Researcher: return "Forscherin";
    case Role::Scientist:  return "Wissenschaftler";
    case Role::Dispatcher: return "Logistikerin";
    case Role::Operations: return "Bautechniker";
    case Role::Quarantine: return "Quarantänebeauftragte";
    case Role::Planner:    return "Krisenplaner";
    }
    return "?";
}

const char* roleAbility(Role role)
{
    switch (role) {
    case Role::Medic:
        return "Behandeln entfernt alle Würfel einer Farbe. Würfel geheilter Seuchen "
               "verschwinden ohne Aktion, sobald sie die Stadt betritt.";
    case Role::Researcher:
        return "Beim Wissensaustausch darf sie jede Stadtkarte abgeben, nicht nur die der aktuellen Stadt.";
    case Role::Scientist:
        return "Braucht nur 4 gleichfarbige Stadtkarten für ein Heilmittel.";
    case Role::Dispatcher:
        return "Darf fremde Figuren bewegen wie die eigene und eine Figur auf das Feld einer anderen ziehen.";
    case Role::Operations:
        return "Baut Forschungsstationen ohne Karte. Einmal pro Zug von einer Station aus in eine "
               "beliebige Stadt, indem er eine beliebige Stadtkarte abwirft.";
    case Role::Quarantine:
        return "In ihrer Stadt und deren Nachbarstädten werden keine Würfel gesetzt und es gibt keine Ausbrüche.";
    case Role::Planner:
        return "Darf als Aktion eine Ereigniskarte vom Ablagestapel nehmen und aufbewahren.";
    }
    return "?";
}

} // namespace seuche
