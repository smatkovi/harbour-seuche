/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 2.6
import Sailfish.Silica 1.0
import "."

// A plain chooser. `entries` is a list of maps carrying either an `id` (cities,
// infection cards) or a `seat` (players); the caller gets that number back.
Page {
    id: page
    allowedOrientations: Orientation.All

    property string title: ""
    property var entries: []
    property var onPicked: null

    SilicaListView {
        anchors.fill: parent
        model: page.entries

        header: PageHeader { title: page.title }

        delegate: ListItem {
            contentHeight: Theme.itemSizeSmall

            Label {
                x: Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                text: modelData.name !== undefined ? modelData.name
                                                   : modelData.role + " — " + modelData.cityName
            }

            onClicked: {
                if (page.onPicked)
                    page.onPicked(modelData.id !== undefined ? modelData.id : modelData.seat)
            }
        }

        VerticalScrollDecorator { }
    }
}
