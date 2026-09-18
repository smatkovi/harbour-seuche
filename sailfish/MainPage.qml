/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 2.6
import Sailfish.Silica 1.0
import "."

// Start page: how many seats sit at the table and how hard the deck is. Every
// seat is played on this device — the game is co-operative, so one person can
// hold all of them, or the phone is passed around.
Page {
    id: page
    objectName: "mainPage"
    allowedOrientations: Orientation.All

    property int seatCount: 4
    property int difficulty: 1

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        PullDownMenu {
            MenuItem {
                text: qsTr("Über Seuche")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }
        }

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader { title: qsTr("Seuche") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Vier Seuchen brechen weltweit aus. Ihr gewinnt gemeinsam, "
                           + "sobald für alle vier ein Heilmittel gefunden ist — und verliert, "
                           + "sobald acht Ausbrüche geschehen sind, ein Würfelvorrat ausgeht "
                           + "oder das Kartendeck leer ist.")
            }

            SectionHeader { text: qsTr("Tisch") }

            Slider {
                width: parent.width
                minimumValue: 2
                maximumValue: 4
                stepSize: 1
                value: page.seatCount
                valueText: qsTr("%1 Personen").arg(value)
                label: qsTr("Sitze an diesem Gerät")
                onValueChanged: page.seatCount = value
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: qsTr("Alle Sitze werden hier gesteuert; die Rollen werden zufällig gezogen. "
                           + "Zu zweit hat jede Person 4 Handkarten, zu dritt 3, zu viert 2.\n"
                           + "Über mehrere Geräte geht es unter „Im Netzwerk spielen\".")
            }

            ComboBox {
                width: parent.width
                label: qsTr("Schwierigkeit")
                currentIndex: page.difficulty
                menu: ContextMenu {
                    MenuItem { text: qsTr("Einführung — 4 Epidemien") }
                    MenuItem { text: qsTr("Normal — 5 Epidemien") }
                    MenuItem { text: qsTr("Heroisch — 6 Epidemien") }
                }
                onCurrentIndexChanged: page.difficulty = currentIndex
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Partie beginnen")
                onClicked: {
                    engine.newGame(page.seatCount, page.difficulty)
                    // roles first: they are dealt at random and may be swapped
                    // freely until the first move
                    pageStack.push(Qt.resolvedUrl("RolePage.qml"))
                }
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Im Netzwerk spielen")
                onClicked: pageStack.push(Qt.resolvedUrl("LanPage.qml"))
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                visible: engine.running && !engine.over
                text: qsTr("Laufende Partie fortsetzen")
                onClicked: pageStack.push(Qt.resolvedUrl("BoardPage.qml"))
            }
        }
    }
}
