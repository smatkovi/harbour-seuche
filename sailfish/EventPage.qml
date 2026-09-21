/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 2.6
import Sailfish.Silica 1.0
import "."

// Event cards cost no action and may be played at any time, by anyone. Cards
// that need a target open a picker first.
Page {
    id: page
    allowedOrientations: Orientation.All

    function pick(title, entries, handler) {
        var picker = pageStack.push(Qt.resolvedUrl("PickPage.qml"),
                                    { title: title, entries: entries })
        picker.picked.connect(handler)
        return picker
    }

    function play(entry) {
        if (entry.needs === "") {
            engine.playEvent(entry.seat, entry.event, -1, -1)
            pageStack.pop()
        } else if (entry.needs === "city") {
            page.pick(qsTr("Wohin?"), engine.cities, function (id) {
                engine.playEvent(entry.seat, entry.event, id, -1)
                pageStack.pop(page)
                pageStack.pop()
            })
        } else if (entry.needs === "pawn") {
            page.pick(qsTr("Wen versetzen?"), engine.players, function (seat) {
                page.pick(qsTr("Wohin?"), engine.cities, function (id) {
                    engine.playEvent(entry.seat, entry.event, id, seat)
                    pageStack.pop(page)
                    pageStack.pop()
                })
            })
        } else if (entry.needs === "infection") {
            page.pick(qsTr("Welche Karte aus dem Spiel nehmen?"),
                      engine.infectionDiscard(), function (id) {
                engine.playEvent(entry.seat, entry.event, id, -1)
                pageStack.pop(page)
                pageStack.pop()
            })
        } else if (entry.needs === "forecast") {
            pageStack.push(Qt.resolvedUrl("ForecastPage.qml"), { seat: entry.seat })
        }
    }

    SilicaListView {
        id: view
        anchors.fill: parent
        model: engine.eventCards()

        header: PageHeader { title: qsTr("Ereigniskarten") }

        delegate: ListItem {
            contentHeight: Theme.itemSizeMedium

            Column {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter

                Label { text: modelData.name }
                Label {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeExtraSmall
                    color: Theme.secondaryColor
                    text: modelData.holder + " · " + modelData.text
                }
            }

            onClicked: page.play(modelData)
        }

        ViewPlaceholder {
            enabled: view.count === 0
            text: qsTr("Niemand hält eine spielbare Ereigniskarte")
        }

        VerticalScrollDecorator { }
    }
}
