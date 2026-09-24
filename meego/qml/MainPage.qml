/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 1.1
import com.nokia.meego 1.0

// Startseite: wie viele Sitze am Tisch sitzen und wie hart das Deck ist. Jeder
// Sitz wird auf diesem Gerät gespielt — das Spiel ist kooperativ, eine Person
// kann alle halten, oder das Telefon wandert herum.
Page {
    id: page
    objectName: "mainPage"
    orientationLock: PageOrientation.Automatic

    property int seatCount: 4
    property int difficulty: 1

    property variant difficultyNames: [
        "Einführung — 4 Epidemien",
        "Normal — 5 Epidemien",
        "Heroisch — 6 Epidemien"
    ]

    tools: ToolBarLayout {
        ToolIcon {
            iconId: "toolbar-view-menu"
            onClicked: mainMenu.open()
        }
    }

    Menu {
        id: mainMenu
        MenuLayout {
            MenuItem {
                text: "Über Seuche"
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }
        }
    }

    SelectionDialog {
        id: difficultyDialog
        titleText: "Schwierigkeit"
        selectedIndex: page.difficulty
        model: ListModel {
            ListElement { name: "Einführung — 4 Epidemien" }
            ListElement { name: "Normal — 5 Epidemien" }
            ListElement { name: "Heroisch — 6 Epidemien" }
        }
        onAccepted: page.difficulty = difficultyDialog.selectedIndex
    }

    Flickable {
        anchors.fill: parent
        contentWidth: width
        contentHeight: column.height + AppTheme.paddingLarge
        clip: true

        Column {
            id: column
            width: page.width
            spacing: AppTheme.paddingMedium

            PageHeader { title: "Seuche" }

            Label {
                x: AppTheme.horizontalPageMargin
                width: parent.width - 2 * AppTheme.horizontalPageMargin
                wrapMode: Text.WordWrap
                color: AppTheme.secondaryColor
                font.pixelSize: AppTheme.fontSizeSmall
                text: "Vier Seuchen brechen weltweit aus. Ihr gewinnt gemeinsam, sobald "
                    + "für alle vier ein Heilmittel gefunden ist — und verliert, sobald "
                    + "acht Ausbrüche geschehen sind, ein Würfelvorrat ausgeht oder das "
                    + "Kartendeck leer ist."
            }

            SectionHeader { text: "Tisch" }

            Label {
                x: AppTheme.horizontalPageMargin
                font.pixelSize: AppTheme.fontSizeSmall
                text: "Sitze an diesem Gerät: " + page.seatCount
            }

            Slider {
                width: parent.width - 2 * AppTheme.horizontalPageMargin
                x: AppTheme.horizontalPageMargin
                minimumValue: 2
                maximumValue: 4
                stepSize: 1
                value: page.seatCount
                valueIndicatorVisible: true
                valueIndicatorText: page.seatCount + " Personen"
                onValueChanged: page.seatCount = Math.round(value)
            }

            Label {
                x: AppTheme.horizontalPageMargin
                width: parent.width - 2 * AppTheme.horizontalPageMargin
                wrapMode: Text.WordWrap
                color: AppTheme.secondaryColor
                font.pixelSize: AppTheme.fontSizeExtraSmall
                text: "Alle Sitze werden hier gesteuert; die Rollen werden zufällig gezogen "
                    + "und lassen sich vor dem ersten Zug frei tauschen. Zu zweit hat jede "
                    + "Person 4 Handkarten, zu dritt 3, zu viert 2.\n"
                    + "Über mehrere Geräte geht es unter „Im Netzwerk spielen“."
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 4 * AppTheme.horizontalPageMargin
                text: page.difficultyNames[page.difficulty]
                onClicked: difficultyDialog.open()
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 4 * AppTheme.horizontalPageMargin
                text: "Partie beginnen"
                onClicked: {
                    engine.newGame(page.seatCount, page.difficulty)
                    // Erst die Rollen: sie werden zufällig gezogen und dürfen
                    // bis zum ersten Zug frei getauscht werden.
                    pageStack.push(Qt.resolvedUrl("RolePage.qml"))
                }
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 4 * AppTheme.horizontalPageMargin
                text: "Im Netzwerk spielen"
                onClicked: pageStack.push(Qt.resolvedUrl("LanPage.qml"))
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 4 * AppTheme.horizontalPageMargin
                visible: engine.running && !engine.over
                text: "Laufende Partie fortsetzen"
                onClicked: pageStack.push(Qt.resolvedUrl("BoardPage.qml"))
            }
        }
    }
}
