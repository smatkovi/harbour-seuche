/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 1.1

// Silicas ListItem: eine Zeile in voller Breite, die sich antippen lässt und
// dabei kurz aufleuchtet. `enabled` schaltet sie stumm, ohne sie zu verbergen.
Item {
    id: item

    property int contentHeight: Theme.itemSizeSmall
    property bool enabled: true
    property alias pressed: area.pressed

    signal clicked()

    width: parent ? parent.width : 0
    height: contentHeight

    Rectangle {
        anchors.fill: parent
        visible: area.pressed && item.enabled
        color: Theme.secondaryHighlightColor
        opacity: 0.45
    }

    MouseArea {
        id: area
        anchors.fill: parent
        enabled: item.enabled
        onClicked: item.clicked()
    }
}
