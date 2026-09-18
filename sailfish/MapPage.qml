/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 2.6
import Sailfish.Silica 1.0
import "."

// Der Spielplan formatfüllend — auf einem Telefon ist die Karte im Querformat
// erst richtig zu lesen. Antippen einer Stadt führt zu denselben Aktionen wie
// auf der Spielseite.
Page {
    id: page
    objectName: "mapPage"
    allowedOrientations: Orientation.All

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
                text: qsTr("Rate %1 · Ausbrüche %2/8 · Deck %3")
                    .arg(engine.infectionRate).arg(engine.outbreaks).arg(engine.playerDeck)
            }
        }
    }
}
