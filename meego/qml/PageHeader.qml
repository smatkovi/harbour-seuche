/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 1.1

// Kopfzeile einer Seite, wie Silicas PageHeader: Titel rechtsbündig, darunter
// bei Bedarf eine Zeile Erläuterung.
Item {
    id: header

    property string title: ""
    property string description: ""

    width: parent ? parent.width : 0
    height: titleLabel.height + (description.length > 0 ? detail.height : 0)
            + 2 * Theme.paddingMedium

    Label {
        id: titleLabel
        anchors.right: parent.right
        anchors.rightMargin: Theme.horizontalPageMargin
        anchors.top: parent.top
        anchors.topMargin: Theme.paddingMedium
        font.pixelSize: Theme.fontSizeLarge
        color: Theme.highlightColor
        text: header.title
    }

    Label {
        id: detail
        anchors.right: parent.right
        anchors.rightMargin: Theme.horizontalPageMargin
        anchors.top: titleLabel.bottom
        width: parent.width - 2 * Theme.horizontalPageMargin
        horizontalAlignment: Text.AlignRight
        wrapMode: Text.WordWrap
        visible: header.description.length > 0
        font.pixelSize: Theme.fontSizeExtraSmall
        color: Theme.secondaryColor
        text: header.description
    }
}
