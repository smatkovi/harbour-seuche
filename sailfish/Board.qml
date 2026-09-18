/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 2.6
import Sailfish.Silica 1.0
import "."

// Der Spielplan als eigener Baustein: einmal klein auf der Spielseite, einmal
// formatfüllend auf der Kartenseite.
//
// Hinter den Städten liegt eine grobe Landmaske (tools/make_world.py, aus den
// FlightGear-Flugplatzdaten). Sie ist Schmuck und Orientierung, keine Regel —
// verbunden sind Städte genau dann, wenn eine Linie sie verbindet.
Item {
    id: root

    property real zoom: 2.6
    readonly property real minZoom: 1.0
    readonly property real maxZoom: 7.0
    // Städtenamen erst, wenn genug Platz zwischen den Städten ist
    readonly property bool showLabels: plan.width > root.width * 2.0
    property bool actionsEnabled: true

    signal cityClicked(int cityId)

    // Die Karte ist längentreu aufgezogen: das Seitenverhältnis ist das der
    // Gradspanne, die die 48 Städte aufspannen.
    readonly property real aspect: 0.3454

    function setZoom(wanted) {
        var next = Math.max(minZoom, Math.min(maxZoom, wanted))
        if (Math.abs(next - zoom) < 0.001)
            return
        var fx = (view.contentX + view.width / 2) / Math.max(1, plan.width)
        var fy = (view.contentY + view.height / 2) / Math.max(1, plan.height)
        zoom = next
        view.contentX = Math.max(0, Math.min(Math.max(0, plan.width - view.width),
                                             fx * plan.width - view.width / 2))
        view.contentY = Math.max(0, Math.min(Math.max(0, plan.height - view.height),
                                             fy * plan.height - view.height / 2))
    }

    function centreOn(cityId) {
        if (cityId < 0 || engine.cities.length === 0)
            return
        var city = engine.cities[cityId]
        view.contentX = Math.max(0, Math.min(Math.max(0, plan.width - view.width),
                                             city.x * plan.width - view.width / 2))
        view.contentY = Math.max(0, Math.min(Math.max(0, plan.height - view.height),
                                             city.y * plan.height - view.height / 2))
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
        contentWidth: plan.width
        contentHeight: plan.height
        boundsBehavior: Flickable.StopAtBounds

        PinchArea {
            width: Math.max(plan.width, view.width)
            height: Math.max(plan.height, view.height)
            property real startZoom: 1

            onPinchStarted: startZoom = root.zoom
            onPinchUpdated: root.setZoom(startZoom * pinch.scale)

            Item {
                id: plan
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

                Canvas {
                    id: links
                    anchors.fill: parent
                    renderStrategy: Canvas.Threaded

                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        ctx.strokeStyle = Style.link
                        ctx.lineWidth = Math.max(1, width / 1000)
                        ctx.lineCap = "round"

                        var list = engine.links
                        for (var i = 0; i < list.length; ++i) {
                            var l = list[i]
                            if (Math.abs(l.x1 - l.x2) > 0.4) {
                                // Über den Pazifik: zwei Stummel über den Rand,
                                // sonst legt sich die Linie quer über alles.
                                var west = l.x1 < l.x2
                                    ? l : { x1: l.x2, y1: l.y2, x2: l.x1, y2: l.y1 }
                                ctx.beginPath()
                                ctx.moveTo(west.x1 * width, west.y1 * height)
                                ctx.lineTo(0, west.y1 * height)
                                ctx.stroke()
                                ctx.beginPath()
                                ctx.moveTo(west.x2 * width, west.y2 * height)
                                ctx.lineTo(width, west.y2 * height)
                                ctx.stroke()
                                continue
                            }
                            ctx.beginPath()
                            ctx.moveTo(l.x1 * width, l.y1 * height)
                            ctx.lineTo(l.x2 * width, l.y2 * height)
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
                            color: Style.colourOf(city.colour)
                            border.width: Math.max(1, width / 9)
                            border.color: city.station ? Style.station : Qt.rgba(0, 0, 0, 0.6)
                        }

                        // Forschungsstation: weißes Quadrat oben rechts
                        Rectangle {
                            visible: city.station
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
                                model: city.cubes.length

                                delegate: Rectangle {
                                    property int count: city.cubes[index]
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

                        // Figuren: links neben der Stadt
                        Column {
                            anchors.right: dot.left
                            anchors.rightMargin: 2
                            anchors.verticalCenter: dot.verticalCenter
                            spacing: 1

                            Repeater {
                                model: city.pawns.length
                                delegate: Rectangle {
                                    width: dot.width * 0.4
                                    height: width
                                    radius: width / 2
                                    color: Style.pawn
                                    border.width: 1
                                    border.color: Qt.rgba(0, 0, 0, 0.65)
                                }
                            }
                        }

                        Rectangle {
                            anchors.top: dot.bottom
                            anchors.topMargin: 2
                            anchors.horizontalCenter: dot.horizontalCenter
                            visible: root.showLabels || cityItem.here || city.cubeCount > 0
                            width: cityLabel.width + 4
                            height: cityLabel.height + 2
                            radius: 2
                            color: Qt.rgba(0, 0, 0, 0.5)

                            Label {
                                id: cityLabel
                                anchors.centerIn: parent
                                font.pixelSize: Theme.fontSizeTiny
                                color: Theme.primaryColor
                                text: city.name
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

    Column {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: Theme.paddingSmall
        spacing: Theme.paddingSmall

        IconButton {
            icon.source: "image://theme/icon-m-add"
            onClicked: root.setZoom(root.zoom * 1.35)
        }
        IconButton {
            icon.source: "image://theme/icon-m-remove"
            onClicked: root.setZoom(root.zoom / 1.35)
        }
        IconButton {
            icon.source: "image://theme/icon-m-location"
            onClicked: root.centreOnTurn()
        }
    }

    Connections {
        target: engine
        onChanged: links.requestPaint()
    }
}
