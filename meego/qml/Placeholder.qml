/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 1.1

// Silicas ViewPlaceholder: der Hinweis in einer leeren Liste.
Label {
    property bool enabled: false

    anchors.centerIn: parent
    width: parent ? parent.width - 4 * AppTheme.horizontalPageMargin : 0
    horizontalAlignment: Text.AlignHCenter
    wrapMode: Text.WordWrap
    visible: enabled
    font.pixelSize: AppTheme.fontSizeSmall
    color: AppTheme.secondaryColor
}
