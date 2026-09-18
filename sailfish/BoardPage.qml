/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 2.6
import Sailfish.Silica 1.0
import "."

// Die Spielseite: Stand, Karte, Handkarten und der eine Knopf, auf den die
// laufende Phase wartet. Die Karte selbst steckt in Board.qml und gibt es unter
// „Karte groß“ noch einmal formatfüllend.
Page {
    id: page
    objectName: "boardPage"
    allowedOrientations: Orientation.All

    function toMainPage() {
        var main = pageStack.find(function (p) { return p.objectName === "mainPage" })
        if (main)
            pageStack.pop(main)
        else
            pageStack.replaceAbove(null, Qt.resolvedUrl("MainPage.qml"))
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        PullDownMenu {
            MenuItem {
                text: qsTr("Neue Partie")
                onClicked: page.toMainPage()
            }
            MenuItem {
                text: qsTr("Protokoll")
                onClicked: pageStack.push(Qt.resolvedUrl("LogPage.qml"))
            }
            MenuItem {
                text: qsTr("Hände und Rollen")
                onClicked: pageStack.push(Qt.resolvedUrl("HandPage.qml"))
            }
            MenuItem {
                text: qsTr("Rollen wählen")
                visible: !engine.rolesLocked
                onClicked: pageStack.push(Qt.resolvedUrl("RolePage.qml"), { showStart: false })
            }
            MenuItem {
                text: qsTr("Ereigniskarte spielen")
                onClicked: pageStack.push(Qt.resolvedUrl("EventPage.qml"))
            }
            MenuItem {
                text: qsTr("Netzwerk")
                onClicked: pageStack.push(Qt.resolvedUrl("LanPage.qml"))
            }
        }

        Column {
            id: column
            width: page.width
            spacing: Theme.paddingSmall

            PageHeader {
                title: engine.phaseText
                description: {
                    if (engine.players.length === 0)
                        return ""
                    var line = engine.players[engine.atTurn].role + " — "
                             + engine.players[engine.atTurn].cityName
                    if (engine.netRole !== "local")
                        line += engine.mayAct ? qsTr(" · du bist dran")
                                              : qsTr(" · anderes Gerät ist dran")
                    return line
                }
            }

            // --- Stand ----------------------------------------------------
            Row {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                spacing: Theme.paddingMedium

                Repeater {
                    model: engine.cures
                    delegate: Row {
                        spacing: Theme.paddingSmall / 2
                        Rectangle {
                            width: Theme.fontSizeSmall
                            height: width
                            radius: modelData.cured ? width / 2 : 0
                            color: Style.colourOf(modelData.colour)
                            opacity: modelData.eradicated ? 0.35 : 1.0
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            font.pixelSize: Theme.fontSizeExtraSmall
                            color: Theme.secondaryColor
                            text: modelData.supply
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryColor
                text: qsTr("Rate %1 · Ausbrüche %2/8 · Stationen %3/6 · Deck %4 · Infektionen %5")
                    .arg(engine.infectionRate).arg(engine.outbreaks).arg(engine.stations)
                    .arg(engine.playerDeck).arg(engine.infectionDeck)
            }

            // --- Karte ----------------------------------------------------
            Board {
                id: boardView
                width: column.width
                height: page.height * 0.42
                actionsEnabled: engine.mayAct
                onCityClicked: pageStack.push(Qt.resolvedUrl("ActionPage.qml"),
                                              { cityId: cityId })
                Component.onCompleted: centreOnTurn()
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Karte groß")
                onClicked: pageStack.push(Qt.resolvedUrl("MapPage.qml"))
            }

            // --- Hand der Person am Zug -----------------------------------
            Flow {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                spacing: Theme.paddingSmall / 2

                Repeater {
                    model: engine.players.length > 0 ? engine.players[engine.atTurn].hand : []

                    delegate: Rectangle {
                        height: Theme.itemSizeExtraSmall * 0.5
                        width: cardLabel.width + Theme.paddingMedium
                        radius: 4
                        color: modelData.colour >= 0 ? Style.colourOf(modelData.colour)
                                                     : Theme.secondaryHighlightColor

                        Label {
                            id: cardLabel
                            anchors.centerIn: parent
                            font.pixelSize: Theme.fontSizeExtraSmall
                            color: "#101010"
                            text: modelData.label
                        }
                    }
                }
            }

            // --- worauf die Phase wartet ----------------------------------
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: Theme.paddingMedium

                Button {
                    visible: engine.phase === "actions"
                    enabled: engine.mayAct
                    text: qsTr("Aktionen")
                    onClicked: pageStack.push(Qt.resolvedUrl("ActionPage.qml"), { cityId: -1 })
                }
                Button {
                    visible: engine.phase === "draw"
                    enabled: engine.mayAct
                    text: qsTr("Karte ziehen (%1)").arg(engine.drawsLeft)
                    onClicked: engine.drawCard()
                }
                Button {
                    visible: engine.phase === "discard"
                    enabled: engine.mayAct
                    text: qsTr("Abwerfen")
                    onClicked: pageStack.push(Qt.resolvedUrl("HandPage.qml"))
                }
                Button {
                    visible: engine.phase === "infect"
                    enabled: engine.mayAct
                    text: qsTr("Infizieren (%1)").arg(engine.infectionsLeft)
                    onClicked: engine.infectCity()
                }
                Button {
                    visible: engine.over
                    text: qsTr("Neue Partie")
                    onClicked: page.toMainPage()
                }
            }

            // die letzten Zeilen des Protokolls, damit der Tisch mitkommt
            Column {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin

                Repeater {
                    model: engine.journal.slice(Math.max(0, engine.journal.length - 3))
                    delegate: Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        font.pixelSize: Theme.fontSizeExtraSmall
                        color: Theme.secondaryColor
                        text: modelData
                    }
                }
            }
        }
    }
}
