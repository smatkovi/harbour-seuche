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
    height: label.height + 2 * AppTheme.paddingMedium

    Label {
        id: label
        anchors.right: parent.right
        anchors.rightMargin: AppTheme.horizontalPageMargin
        anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: AppTheme.fontSizeSmall
        color: AppTheme.highlightColor
    }

    Rectangle {
        anchors.left: parent.left
        anchors.leftMargin: AppTheme.horizontalPageMargin
        anchors.right: label.left
        anchors.rightMargin: AppTheme.paddingMedium
        anchors.verticalCenter: label.verticalCenter
        height: 1
        color: AppTheme.secondaryHighlightColor
    }
}
