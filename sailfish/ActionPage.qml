/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 2.6
import Sailfish.Silica 1.0
import "."

// Everything the core allows, either for one city (tapped on the board) or the
// whole list. The page only shows labels and sends back an index.
Page {
    id: page
    allowedOrientations: Orientation.All

    // -1 lists every legal action
    property int cityId: -1

    SilicaListView {
        id: view
        anchors.fill: parent
        model: page.cityId >= 0 ? engine.actionsForCity(page.cityId) : engine.allActions()

        header: PageHeader {
            title: page.cityId >= 0 ? engine.cityName(page.cityId) : qsTr("Aktionen")
            description: qsTr("%1 Aktionen übrig").arg(engine.actionsLeft)
        }

        delegate: ListItem {
            contentHeight: Theme.itemSizeSmall

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                truncationMode: TruncationMode.Fade
                text: modelData.label
            }

            onClicked: {
                engine.run(modelData.index)
                pageStack.pop()
            }
        }

        ViewPlaceholder {
            enabled: view.count === 0
            text: page.cityId >= 0 ? qsTr("Hier ist gerade nichts möglich")
                                   : qsTr("Keine Aktion möglich")
        }

        VerticalScrollDecorator { }
    }
}
