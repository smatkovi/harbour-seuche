/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 1.1
import com.nokia.meego 1.0

// Der Spielplan formatfüllend — auf einem Telefon ist die Karte im Querformat
// erst richtig zu lesen. Antippen einer Stadt führt zu denselben Aktionen wie
// auf der Spielseite.
Page {
    id: page
    objectName: "mapPage"
    orientationLock: PageOrientation.Automatic

    tools: ToolBarLayout {
        ToolIcon { iconId: "toolbar-back"; onClicked: pageStack.pop() }
    }

    Board {
        id: board
        anchors.fill: parent
        actionsEnabled: engine.mayAct
        zoom: 3.2
        onCityClicked: pageStack.push(Qt.resolvedUrl("ActionPage.qml"), { cityId: cityId })
        Component.onCompleted: centreOnTurn()
    }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: Theme.paddingMedium
        width: status.width + Theme.paddingLarge
        height: status.height + Theme.paddingMedium
        radius: Theme.paddingSmall
        color: Qt.rgba(0, 0, 0, 0.55)

        Column {
            id: status
            anchors.centerIn: parent

            Label {
                font.pixelSize: Theme.fontSizeSmall
                text: engine.phaseText
            }
            Label {
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryColor
                text: "Rate " + engine.infectionRate + " · Ausbrüche " + engine.outbreaks
                    + "/8 · Deck " + engine.playerDeck
            }
        }
    }
}
