/*
    Copyright (C) 2026 smatkovi

    SPDX-License-Identifier: GPL-3.0-or-later
*/
#pragma once

#include "core/Action.h"
#include "core/State.h"

#include <QVariantMap>

// Wire format for the LAN mode. In this game everything is open except the
// order of the two face-down decks (spec/regeln.md §9), so a guest gets the
// whole state and only counts for the decks — there is nothing to hide and
// nothing to cheat with.
//
// The host stays the only device that ever changes the game: guests send an
// intent, the host applies it through the rule core and sends the new state.
namespace seuche {
namespace wire {

QVariantMap toSnapshot(const Game &game);

// Rebuilds a guest's mirror. The decks come back as the right number of blank
// cards, so every count and every legality check is right while the order stays
// where it belongs, on the host.
bool fromSnapshot(const QVariantMap &snapshot, Game &game);

QVariantMap toVariant(const Action &action);
Action actionFromVariant(const QVariantMap &map);

QVariantMap toVariant(const EventPlay &play);
EventPlay eventFromVariant(const QVariantMap &map);

} // namespace wire
} // namespace seuche
