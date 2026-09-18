/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 2.6
import Sailfish.Silica 1.0
import "."

// The table: the board, the hand of whoever is at turn, and one button for the
// step the phase is waiting for. Tapping a city offers exactly the actions the
// core says are legal there, so no rule is decided in QML.
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

            // --- status ---------------------------------------------------
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

            // --- board ----------------------------------------------------
            Item {
                id: mapBox
                width: column.width
                height: page.height * 0.44

                Rectangle {
                    anchors.fill: parent
                    color: Style.sea
                }

                Flickable {
                    id: mapView
                    anchors.fill: parent
                    clip: true
                    contentWidth: board.width
                    contentHeight: board.height
                    boundsBehavior: Flickable.StopAtBounds

                    Item {
                        id: board
                        width: mapBox.width * 2.6
                        height: Math.max(mapBox.height, board.width * 0.42)

                        Canvas {
                            id: links
                            anchors.fill: parent
                            onPaint: {
                                var ctx = getContext("2d")
                                ctx.clearRect(0, 0, width, height)
                                ctx.strokeStyle = Style.link
                                ctx.lineWidth = Math.max(1, width / 900)
                                var list = engine.links
                                for (var i = 0; i < list.length; ++i) {
                                    ctx.beginPath()
                                    ctx.moveTo(list[i].x1 * width, list[i].y1 * height)
                                    ctx.lineTo(list[i].x2 * width, list[i].y2 * height)
                                    ctx.stroke()
                                }
                            }
                            onWidthChanged: requestPaint()
                            onHeightChanged: requestPaint()
                        }

                        Repeater {
                            model: engine.cities

                            delegate: Item {
                                id: cityItem
                                property var city: modelData
                                width: Theme.itemSizeExtraSmall * 0.55
                                height: width
                                x: city.x * board.width - width / 2
                                y: city.y * board.height - height / 2

                                Rectangle {
                                    id: dot
                                    anchors.fill: parent
                                    radius: width / 2
                                    color: Style.colourOf(city.colour)
                                    border.width: city.station ? Math.max(2, width / 8) : 0
                                    border.color: Style.station
                                }

                                // cubes, one small square per cube
                                Row {
                                    anchors.bottom: dot.top
                                    anchors.horizontalCenter: dot.horizontalCenter
                                    anchors.bottomMargin: 1
                                    spacing: 1

                                    Repeater {
                                        model: city.cubes.length
                                        delegate: Row {
                                            property int colourIndex: index
                                            spacing: 1
                                            Repeater {
                                                model: city.cubes[colourIndex]
                                                delegate: Rectangle {
                                                    width: Math.max(3, dot.width / 3)
                                                    height: width
                                                    color: Style.colourOf(colourIndex)
                                                    border.width: 1
                                                    border.color: Qt.rgba(0, 0, 0, 0.5)
                                                }
                                            }
                                        }
                                    }
                                }

                                // pawns
                                Row {
                                    anchors.top: dot.bottom
                                    anchors.horizontalCenter: dot.horizontalCenter
                                    anchors.topMargin: 1
                                    spacing: 1

                                    Repeater {
                                        model: city.pawns.length
                                        delegate: Rectangle {
                                            width: Math.max(3, dot.width / 3)
                                            height: width
                                            radius: width / 2
                                            color: Style.pawn
                                        }
                                    }
                                }

                                Label {
                                    anchors.top: dot.bottom
                                    anchors.topMargin: dot.width / 2
                                    anchors.horizontalCenter: dot.horizontalCenter
                                    font.pixelSize: Theme.fontSizeTiny
                                    color: Theme.primaryColor
                                    text: city.name
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    anchors.margins: -dot.width / 3
                                    enabled: engine.mayAct
                                    onClicked: pageStack.push(Qt.resolvedUrl("ActionPage.qml"),
                                                              { cityId: cityItem.city.id })
                                }
                            }
                        }
                    }
                }
            }

            // --- hand of the player at turn -------------------------------
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

            // --- what the phase waits for ---------------------------------
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

            // the last few lines of the journal, so the table can follow along
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

    Connections {
        target: engine
        onChanged: links.requestPaint()
    }
}
