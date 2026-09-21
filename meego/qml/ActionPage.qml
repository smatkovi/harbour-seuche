/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 1.1
import com.nokia.meego 1.0

// Alles, was der Kern erlaubt, entweder für eine Stadt (auf dem Plan getippt)
// oder die ganze Liste. Die Seite zeigt nur Beschriftungen und schickt einen
// Index zurück.
Page {
    id: page
    orientationLock: PageOrientation.Automatic

    // -1 listet jede erlaubte Aktion
    property int cityId: -1

    tools: ToolBarLayout {
        ToolIcon { iconId: "toolbar-back"; onClicked: pageStack.pop() }
    }

    ListView {
        id: view
        anchors.fill: parent
        clip: true
        model: page.cityId >= 0 ? engine.actionsForCity(page.cityId) : engine.allActions()

        header: PageHeader {
            title: page.cityId >= 0 ? engine.cityName(page.cityId) : "Aktionen"
            description: engine.actionsLeft + " Aktionen übrig"
        }

        delegate: ListItem {
            contentHeight: Theme.itemSizeSmall

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                elide: Text.ElideRight
                font.pixelSize: Theme.fontSizeSmall
                text: modelData.label
            }

            onClicked: {
                engine.run(modelData.index)
                pageStack.pop()
            }
        }
    }

    Placeholder {
        enabled: view.count === 0
        text: page.cityId >= 0 ? "Hier ist gerade nichts möglich" : "Keine Aktion möglich"
    }

    ScrollDecorator { flickableItem: view }
}
