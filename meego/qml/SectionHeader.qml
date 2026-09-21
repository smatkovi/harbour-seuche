/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 1.1

Item {
    id: section
    property alias text: label.text

    width: parent ? parent.width : 0
    height: label.height + 2 * Theme.paddingMedium

    Label {
        id: label
        anchors.right: parent.right
        anchors.rightMargin: Theme.horizontalPageMargin
        anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: Theme.fontSizeSmall
        color: Theme.highlightColor
    }

    Rectangle {
        anchors.left: parent.left
        anchors.leftMargin: Theme.horizontalPageMargin
        anchors.right: label.left
        anchors.rightMargin: Theme.paddingMedium
        anchors.verticalCenter: label.verticalCenter
        height: 1
        color: Theme.secondaryHighlightColor
    }
}
