/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 2.6
import Sailfish.Silica 1.0
import "."

// What has happened, newest last.
Page {
    allowedOrientations: Orientation.All

    SilicaListView {
        anchors.fill: parent
        header: PageHeader { title: qsTr("Protokoll") }
        model: engine.journal

        delegate: Label {
            x: Theme.horizontalPageMargin
            width: parent.width - 2 * Theme.horizontalPageMargin
            wrapMode: Text.WordWrap
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.primaryColor
            text: modelData
        }

        VerticalScrollDecorator { }
    }
}
