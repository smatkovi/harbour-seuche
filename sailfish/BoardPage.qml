/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 2.6
import Sailfish.Silica 1.0
import "."

// Der Spielplan. Die Karte zeichnet sich aus den Daten des Kerns: hinter den
// Städten liegt je Farbe eine weiche Fläche (die Hülle ihrer zwölf Städte),
// darüber die Verbindungen. Die drei Verbindungen über den Pazifik laufen nicht
// quer durch das ganze Bild, sondern als Stummel über den Rand — sonst legen
// sie sich über alles andere. Zoomen geht mit zwei Fingern oder den Knöpfen.
Page {
    id: page
    objectName: "boardPage"
    allowedOrientations: Orientation.All

    property real zoom: 2.6
    readonly property real minZoom: 1.0
    readonly property real maxZoom: 6.0
    readonly property bool labelsVisible: zoom >= 2.0

    function setZoom(wanted) {
        var next = Math.max(minZoom, Math.min(maxZoom, wanted))
        if (Math.abs(next - zoom) < 0.001)
            return
        // keep whatever sits in the middle of the view in the middle
        var fx = (mapView.contentX + mapView.width / 2) / Math.max(1, board.width)
        var fy = (mapView.contentY + mapView.height / 2) / Math.max(1, board.height)
        zoom = next
        mapView.contentX = Math.max(0, Math.min(board.width - mapView.width,
                                                fx * board.width - mapView.width / 2))
        mapView.contentY = Math.max(0, Math.min(board.height - mapView.height,
                                                fy * board.height - mapView.height / 2))
    }

    function centreOnTurn() {
        if (engine.players.length === 0)
            return
        var city = engine.cities[engine.players[engine.atTurn].city]
        mapView.contentX = Math.max(0, Math.min(board.width - mapView.width,
                                                city.x * board.width - mapView.width / 2))
        mapView.contentY = Math.max(0, Math.min(board.height - mapView.height,
                                                city.y * board.height - mapView.height / 2))
    }

    function toMainPage() {
        var main = pageStack.find(function (p) { return p.objectName === "mainPage" })
        if (main)
            pageStack.pop(main)
        else
            pageStack.replaceAbove(null, Qt.resolvedUrl("MainPage.qml"))
    }

    Component.onCompleted: centreOnTurn()

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
                height: page.height * 0.46

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

                    PinchArea {
                        width: Math.max(board.width, mapView.width)
                        height: Math.max(board.height, mapView.height)
                        property real startZoom: 1

                        onPinchStarted: startZoom = page.zoom
                        onPinchUpdated: page.setZoom(startZoom * pinch.scale)

                        Item {
                            id: board
                            width: mapBox.width * page.zoom
                            height: width * 0.46

                            Canvas {
                                id: canvas
                                anchors.fill: parent
                                renderStrategy: Canvas.Threaded

                                // Convex hull, so each colour gets one soft area
                                // instead of twelve loose dots.
                                function hull(points) {
                                    if (points.length < 3)
                                        return points
                                    var sorted = points.slice().sort(function (a, b) {
                                        return a.x === b.x ? a.y - b.y : a.x - b.x
                                    })
                                    var cross = function (o, a, b) {
                                        return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x)
                                    }
                                    var lower = []
                                    for (var i = 0; i < sorted.length; ++i) {
                                        while (lower.length >= 2 && cross(lower[lower.length - 2],
                                                                          lower[lower.length - 1],
                                                                          sorted[i]) <= 0)
                                            lower.pop()
                                        lower.push(sorted[i])
                                    }
                                    var upper = []
                                    for (var j = sorted.length - 1; j >= 0; --j) {
                                        while (upper.length >= 2 && cross(upper[upper.length - 2],
                                                                          upper[upper.length - 1],
                                                                          sorted[j]) <= 0)
                                            upper.pop()
                                        upper.push(sorted[j])
                                    }
                                    lower.pop()
                                    upper.pop()
                                    return lower.concat(upper)
                                }

                                onPaint: {
                                    var ctx = getContext("2d")
                                    ctx.clearRect(0, 0, width, height)
                                    var cities = engine.cities
                                    if (cities.length === 0)
                                        return

                                    // 1. one soft area per disease colour
                                    var pad = Math.max(6, width * 0.012)
                                    ctx.lineJoin = "round"
                                    ctx.lineCap = "round"
                                    for (var colour = 0; colour < 4; ++colour) {
                                        var points = []
                                        for (var c = 0; c < cities.length; ++c) {
                                            if (cities[c].colour === colour)
                                                points.push({ x: cities[c].x * width,
                                                              y: cities[c].y * height })
                                        }
                                        var ring = canvas.hull(points)
                                        if (ring.length < 3)
                                            continue
                                        ctx.beginPath()
                                        ctx.moveTo(ring[0].x, ring[0].y)
                                        for (var r = 1; r < ring.length; ++r)
                                            ctx.lineTo(ring[r].x, ring[r].y)
                                        ctx.closePath()
                                        ctx.fillStyle = Qt.rgba(0, 0, 0, 0)
                                        ctx.globalAlpha = 0.16
                                        ctx.fillStyle = Style.colourOf(colour)
                                        ctx.fill()
                                        // stroking with a fat round pen inflates
                                        // the hull and rounds its corners
                                        ctx.globalAlpha = 0.16
                                        ctx.strokeStyle = Style.colourOf(colour)
                                        ctx.lineWidth = pad * 2
                                        ctx.stroke()
                                        ctx.globalAlpha = 1.0
                                    }

                                    // 2. the connections
                                    var links = engine.links
                                    ctx.strokeStyle = Style.link
                                    ctx.lineWidth = Math.max(1, width / 1100)
                                    for (var i = 0; i < links.length; ++i) {
                                        var l = links[i]
                                        if (Math.abs(l.x1 - l.x2) > 0.4) {
                                            // across the Pacific: two stubs over
                                            // the edge instead of one line over
                                            // the whole map
                                            var west = l.x1 < l.x2 ? l : { x1: l.x2, y1: l.y2, x2: l.x1, y2: l.y1 }
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

                                    // constant on screen: zooming spreads the
                                    // cities apart instead of blowing them up
                                    width: Theme.itemSizeExtraSmall * 0.5
                                    height: width
                                    x: city.x * board.width - width / 2
                                    y: city.y * board.height - height / 2
                                    z: here ? 2 : 1

                                    Rectangle {
                                        id: halo
                                        anchors.centerIn: dot
                                        width: dot.width * 1.9
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
                                        border.width: Math.max(1, width / 10)
                                        border.color: city.station ? Style.station
                                                                   : Qt.rgba(0, 0, 0, 0.55)
                                    }

                                    // one badge per colour that lies here
                                    Row {
                                        anchors.bottom: dot.top
                                        anchors.bottomMargin: 2
                                        anchors.horizontalCenter: dot.horizontalCenter
                                        spacing: 2

                                        Repeater {
                                            model: city.cubes.length

                                            delegate: Rectangle {
                                                property int count: city.cubes[index]
                                                visible: count > 0
                                                width: visible ? dot.width * 0.5 : 0
                                                height: width
                                                radius: 2
                                                color: Style.colourOf(index)
                                                border.width: 1
                                                border.color: Qt.rgba(0, 0, 0, 0.6)

                                                Label {
                                                    anchors.centerIn: parent
                                                    visible: parent.count > 1
                                                    font.pixelSize: parent.width * 0.8
                                                    font.bold: true
                                                    color: "#101010"
                                                    text: parent.count
                                                }
                                            }
                                        }
                                    }

                                    // pawns
                                    Row {
                                        anchors.top: dot.bottom
                                        anchors.topMargin: 1
                                        anchors.horizontalCenter: dot.horizontalCenter
                                        spacing: 1

                                        Repeater {
                                            model: city.pawns.length
                                            delegate: Rectangle {
                                                width: dot.width * 0.34
                                                height: width
                                                radius: width / 2
                                                color: Style.pawn
                                                border.width: 1
                                                border.color: Qt.rgba(0, 0, 0, 0.6)
                                            }
                                        }
                                    }

                                    Rectangle {
                                        anchors.top: dot.bottom
                                        anchors.topMargin: dot.width * 0.45
                                        anchors.horizontalCenter: dot.horizontalCenter
                                        visible: page.labelsVisible
                                        width: cityLabel.width + 4
                                        height: cityLabel.height + 2
                                        radius: 2
                                        color: Qt.rgba(0, 0, 0, 0.45)

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
                                        width: dot.width * 2.2
                                        height: width
                                        enabled: engine.mayAct
                                        onClicked: pageStack.push(Qt.resolvedUrl("ActionPage.qml"),
                                                                  { cityId: cityItem.city.id })
                                    }
                                }
                            }
                        }
                    }
                }

                // zoom and a way back to the pawn at turn
                Column {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.margins: Theme.paddingSmall
                    spacing: Theme.paddingSmall

                    IconButton {
                        icon.source: "image://theme/icon-m-add"
                        onClicked: page.setZoom(page.zoom * 1.35)
                    }
                    IconButton {
                        icon.source: "image://theme/icon-m-remove"
                        onClicked: page.setZoom(page.zoom / 1.35)
                    }
                    IconButton {
                        icon.source: "image://theme/icon-m-location"
                        onClicked: page.centreOnTurn()
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
        onChanged: canvas.requestPaint()
    }
}
