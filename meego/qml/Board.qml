/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 1.1
import harbour.seuche 1.0

// Der Spielplan als eigener Baustein: einmal klein auf der Spielseite, einmal
// formatfüllend auf der Kartenseite. Aufbau wie sailfish/Board.qml, mit zwei
// Unterschieden, die QtQuick 1.1 erzwingt: die Verbindungslinien zeichnet
// LinkItem aus C++ statt eines Canvas, und `property` gibt es hier
// noch nicht.
Item {
    id: root

    property real zoom: 2.6
    property real minZoom: 1.0
    property real maxZoom: 7.0
    // Städtenamen erst, wenn genug Platz zwischen den Städten ist
    property bool showLabels: plan.width > root.width * 2.0
    property bool actionsEnabled: true

    // Die Karte ist längentreu aufgezogen: das Seitenverhältnis ist das der
    // Gradspanne, die die 48 Städte aufspannen.
    property real aspect: 0.3454

    // Die Städte sind auf 0..1 normiert, also sitzen San Francisco, Sydney,
    // Sankt Petersburg und Buenos Aires genau auf der Kante des
    // Kartenrechtecks. Würfel und Figuren werden aber neben die Stadt
    // gezeichnet und fielen damit aus dem sichtbaren Bereich — und antippen
    // ließen sich die Städte auch nicht. Deshalb ist die Fläche zum Schieben
    // ringsum breiter als die Karte.
    property real pad: Theme.itemSizeExtraSmall * 0.9

    signal cityClicked(int cityId)

    function setZoom(wanted) {
        var next = Math.max(minZoom, Math.min(maxZoom, wanted))
        if (Math.abs(next - zoom) < 0.001)
            return
        var fx = (view.contentX + view.width / 2) / Math.max(1, sheet.width)
        var fy = (view.contentY + view.height / 2) / Math.max(1, sheet.height)
        zoom = next
        view.contentX = Math.max(0, Math.min(Math.max(0, sheet.width - view.width),
                                             fx * sheet.width - view.width / 2))
        view.contentY = Math.max(0, Math.min(Math.max(0, sheet.height - view.height),
                                             fy * sheet.height - view.height / 2))
    }

    function centreOn(cityId) {
        if (cityId < 0 || engine.cities.length === 0)
            return
        var city = engine.cities[cityId]
        view.contentX = Math.max(0, Math.min(Math.max(0, sheet.width - view.width),
                                             city.x * plan.width + root.pad - view.width / 2))
        view.contentY = Math.max(0, Math.min(Math.max(0, sheet.height - view.height),
                                             city.y * plan.height + root.pad - view.height / 2))
    }

    function centreOnTurn() {
        if (engine.players.length > 0)
            centreOn(engine.players[engine.atTurn].city)
    }

    Rectangle {
        anchors.fill: parent
        color: Style.sea
    }

    Flickable {
        id: view
        anchors.fill: parent
        clip: true
        contentWidth: sheet.width
        contentHeight: sheet.height
        boundsBehavior: Flickable.StopAtBounds

        PinchArea {
            width: Math.max(sheet.width, view.width)
            height: Math.max(sheet.height, view.height)
            property real startZoom: 1

            onPinchStarted: startZoom = root.zoom
            onPinchUpdated: root.setZoom(startZoom * pinch.scale)

            Item {
                id: sheet
                width: plan.width + 2 * root.pad
                height: plan.height + 2 * root.pad

                Item {
                    id: plan
                    x: root.pad
                    y: root.pad
                    width: root.width * root.zoom
                    height: width * root.aspect

                    Image {
                        anchors.fill: parent
                        source: "world.png"
                        fillMode: Image.Stretch
                        smooth: true
                        opacity: 0.55
                        asynchronous: true
                    }

                    LinkItem {
                        anchors.fill: parent
                        links: engine.links
                        colour: Style.link
                    }

                    Repeater {
                        model: engine.cities

                        delegate: Item {
                            id: cityItem
                            property variant city: modelData
                            property bool here: engine.players.length > 0
                                && engine.players[engine.atTurn].city === city.id

                            // auf dem Schirm immer gleich groß: Zoomen zieht die
                            // Städte auseinander, statt alles aufzublasen
                            width: Theme.itemSizeExtraSmall * 0.46
                            height: width
                            x: city.x * plan.width - width / 2
                            y: city.y * plan.height - height / 2
                            z: here ? 3 : (city.cubeCount > 0 ? 2 : 1)

                            Rectangle {
                                anchors.centerIn: dot
                                width: dot.width * 2.0
                                height: width
                                radius: width / 2
                                visible: cityItem.here
                                color: "transparent"
                                border.width: Math.max(2, dot.width / 6)
                                border.color: Theme.highlightColor
                            }

                            Rectangle {
                                id: dot
                                anchors.fill: parent
                                radius: width / 2
                                color: Style.colourOf(cityItem.city.colour)
                                border.width: Math.max(1, width / 9)
                                border.color: cityItem.city.station ? Style.station
                                                                    : Qt.rgba(0, 0, 0, 0.6)
                            }

                            // Forschungsstation: weißes Quadrat oben rechts
                            Rectangle {
                                visible: cityItem.city.station
                                width: dot.width * 0.42
                                height: width
                                anchors.left: dot.right
                                anchors.bottom: dot.top
                                anchors.margins: -width / 3
                                color: Style.station
                                border.width: 1
                                border.color: Qt.rgba(0, 0, 0, 0.6)
                            }

                            // Würfel: ein Plättchen je Farbe, rechts neben der Stadt
                            Column {
                                anchors.left: dot.right
                                anchors.leftMargin: 2
                                anchors.verticalCenter: dot.verticalCenter
                                spacing: 1

                                Repeater {
                                    model: cityItem.city.cubes.length

                                    delegate: Rectangle {
                                        property int count: cityItem.city.cubes[index]
                                        visible: count > 0
                                        width: visible ? dot.width * 0.55 : 0
                                        height: width
                                        radius: 2
                                        color: Style.colourOf(index)
                                        border.width: 1
                                        border.color: Qt.rgba(0, 0, 0, 0.65)

                                        Label {
                                            anchors.centerIn: parent
                                            visible: parent.count > 1
                                            font.pixelSize: parent.width * 0.85
                                            font.bold: true
                                            color: "#101010"
                                            text: parent.count
                                        }
                                    }
                                }
                            }

                            // Figuren: links neben der Stadt, in der Farbe ihrer Rolle
                            Column {
                                anchors.right: dot.left
                                anchors.rightMargin: 2
                                anchors.verticalCenter: dot.verticalCenter
                                spacing: 1

                                Repeater {
                                    model: cityItem.city.pawns
                                    delegate: Rectangle {
                                        property int seat: modelData
                                        property bool onTurn: seat === engine.atTurn
                                        width: dot.width * (onTurn ? 0.52 : 0.44)
                                        height: width
                                        radius: width / 2
                                        color: seat < engine.players.length
                                            ? Style.roleColourOf(engine.players[seat].roleId)
                                            : Style.pawn
                                        border.width: onTurn ? Math.max(1, width / 5) : 1
                                        border.color: onTurn ? Theme.highlightColor
                                                             : Qt.rgba(0, 0, 0, 0.65)
                                    }
                                }
                            }

                            Rectangle {
                                anchors.top: dot.bottom
                                anchors.topMargin: 2
                                anchors.horizontalCenter: dot.horizontalCenter
                                visible: root.showLabels || cityItem.here
                                         || cityItem.city.cubeCount > 0
                                width: cityLabel.width + 4
                                height: cityLabel.height + 2
                                radius: 2
                                color: Qt.rgba(0, 0, 0, 0.5)

                                Label {
                                    id: cityLabel
                                    anchors.centerIn: parent
                                    font.pixelSize: Theme.fontSizeTiny
                                    color: Theme.primaryColor
                                    text: cityItem.city.name
                                }
                            }

                            MouseArea {
                                anchors.centerIn: dot
                                width: dot.width * 2.4
                                height: width
                                enabled: root.actionsEnabled
                                onClicked: root.cityClicked(cityItem.city.id)
                            }
                        }
                    }
                }
            }
        }
    }

    Column {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: Theme.paddingSmall
        spacing: Theme.paddingSmall

        Rectangle {
            width: Theme.itemSizeExtraSmall * 0.7
            height: width
            radius: width / 2
            color: Qt.rgba(0, 0, 0, 0.55)
            Label {
                anchors.centerIn: parent
                text: "+"
                font.pixelSize: Theme.fontSizeLarge
            }
            MouseArea {
                anchors.fill: parent
                onClicked: root.setZoom(root.zoom * 1.35)
            }
        }
        Rectangle {
            width: Theme.itemSizeExtraSmall * 0.7
            height: width
            radius: width / 2
            color: Qt.rgba(0, 0, 0, 0.55)
            Label {
                anchors.centerIn: parent
                text: "−"
                font.pixelSize: Theme.fontSizeLarge
            }
            MouseArea {
                anchors.fill: parent
                onClicked: root.setZoom(root.zoom / 1.35)
            }
        }
        Rectangle {
            width: Theme.itemSizeExtraSmall * 0.7
            height: width
            radius: width / 2
            color: Qt.rgba(0, 0, 0, 0.55)
            Label {
                anchors.centerIn: parent
                text: "⌖"
                font.pixelSize: Theme.fontSizeMedium
            }
            MouseArea {
                anchors.fill: parent
                onClicked: root.centreOnTurn()
            }
        }
    }
}
