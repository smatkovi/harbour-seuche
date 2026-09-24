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
            + 2 * AppTheme.paddingMedium

    Label {
        id: titleLabel
        anchors.right: parent.right
        anchors.rightMargin: AppTheme.horizontalPageMargin
        anchors.top: parent.top
        anchors.topMargin: AppTheme.paddingMedium
        font.pixelSize: AppTheme.fontSizeLarge
        color: AppTheme.highlightColor
        text: header.title
    }

    Label {
        id: detail
        anchors.right: parent.right
        anchors.rightMargin: AppTheme.horizontalPageMargin
        anchors.top: titleLabel.bottom
        width: parent.width - 2 * AppTheme.horizontalPageMargin
        horizontalAlignment: Text.AlignRight
        wrapMode: Text.WordWrap
        visible: header.description.length > 0
        font.pixelSize: AppTheme.fontSizeExtraSmall
        color: AppTheme.secondaryColor
        text: header.description
    }
}
