/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 2.6
import Sailfish.Silica 1.0
import "."

// Prognose: the top six infection cards. Tap them in the order they should come
// off the deck; the first tap is the next card to be drawn.
Page {
    id: page
    allowedOrientations: Orientation.All

    property int seat: 0
    property var cards: engine.forecastCards()
    property var chosen: []

    function pick(id) {
        var list = page.chosen.slice()
        list.push(id)
        page.chosen = list
        if (page.chosen.length === page.cards.length) {
            engine.playForecast(page.seat, page.chosen)
            pageStack.pop()
        }
    }

    function taken(id) {
        for (var i = 0; i < page.chosen.length; ++i) {
            if (page.chosen[i] === id)
                return i + 1
        }
        return 0
    }

    SilicaListView {
        anchors.fill: parent
        model: page.cards

        header: PageHeader {
            title: qsTr("Prognose")
            description: qsTr("In Ziehreihenfolge antippen")
        }

        delegate: ListItem {
            contentHeight: Theme.itemSizeSmall
            enabled: page.taken(modelData.id) === 0

            Rectangle {
                x: Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                width: Theme.paddingSmall
                height: parent.height * 0.6
                color: Style.colourOf(modelData.colour)
            }

            Label {
                x: Theme.horizontalPageMargin + Theme.paddingLarge
                anchors.verticalCenter: parent.verticalCenter
                text: modelData.name
                color: enabled ? Theme.primaryColor : Theme.secondaryColor
            }

            Label {
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                visible: page.taken(modelData.id) > 0
                color: Theme.highlightColor
                text: page.taken(modelData.id)
            }

            onClicked: page.pick(modelData.id)
        }
    }
}
