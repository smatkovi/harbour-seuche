/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 1.1
import com.nokia.meego 1.0

// Prognose: die obersten sechs Infektionskarten. In der Reihenfolge antippen,
// in der sie vom Stapel kommen sollen; der erste Tipp ist die nächste Karte.
Page {
    id: page
    orientationLock: PageOrientation.Automatic

    property int seat: 0
    property variant cards: engine.forecastCards()
    property variant chosen: []

    tools: ToolBarLayout {
        ToolIcon { iconId: "toolbar-back"; onClicked: pageStack.pop() }
    }

    function pick(id) {
        var list = []
        for (var i = 0; i < page.chosen.length; ++i)
            list.push(page.chosen[i])
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

    ListView {
        id: view
        anchors.fill: parent
        clip: true
        model: page.cards

        header: PageHeader {
            title: "Prognose"
            description: "In Ziehreihenfolge antippen"
        }

        delegate: ListItem {
            id: row
            contentHeight: AppTheme.itemSizeSmall
            enabled: page.taken(modelData.id) === 0

            Rectangle {
                x: AppTheme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                width: AppTheme.paddingSmall
                height: parent.height * 0.6
                color: Style.colourOf(modelData.colour)
            }

            Label {
                x: AppTheme.horizontalPageMargin + AppTheme.paddingLarge
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: AppTheme.fontSizeSmall
                text: modelData.name
                color: row.enabled ? AppTheme.primaryColor : AppTheme.secondaryColor
            }

            Label {
                anchors.right: parent.right
                anchors.rightMargin: AppTheme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                visible: page.taken(modelData.id) > 0
                color: AppTheme.highlightColor
                text: page.taken(modelData.id)
            }

            onClicked: page.pick(modelData.id)
        }
    }

    ScrollDecorator { flickableItem: view }
}
