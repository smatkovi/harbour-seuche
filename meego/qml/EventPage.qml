/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 1.1
import com.nokia.meego 1.0

// Ereigniskarten kosten keine Aktion und dürfen jederzeit von jedem gespielt
// werden. Karten, die ein Ziel brauchen, öffnen zuerst eine Auswahl.
Page {
    id: page
    orientationLock: PageOrientation.Automatic

    tools: ToolBarLayout {
        ToolIcon { iconId: "toolbar-back"; onClicked: pageStack.pop() }
    }

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
            page.pick("Wohin?", engine.cities, function (id) {
                engine.playEvent(entry.seat, entry.event, id, -1)
                pageStack.pop(page)
                pageStack.pop()
            })
        } else if (entry.needs === "pawn") {
            page.pick("Wen versetzen?", engine.players, function (seat) {
                page.pick("Wohin?", engine.cities, function (id) {
                    engine.playEvent(entry.seat, entry.event, id, seat)
                    pageStack.pop(page)
                    pageStack.pop()
                })
            })
        } else if (entry.needs === "infection") {
            page.pick("Welche Karte aus dem Spiel nehmen?",
                      engine.infectionDiscard(), function (id) {
                engine.playEvent(entry.seat, entry.event, id, -1)
                pageStack.pop(page)
                pageStack.pop()
            })
        } else if (entry.needs === "forecast") {
            pageStack.push(Qt.resolvedUrl("ForecastPage.qml"), { seat: entry.seat })
        }
    }

    ListView {
        id: view
        anchors.fill: parent
        clip: true
        model: engine.eventCards()

        header: PageHeader { title: "Ereigniskarten" }

        delegate: ListItem {
            contentHeight: AppTheme.itemSizeMedium

            Column {
                x: AppTheme.horizontalPageMargin
                width: parent.width - 2 * AppTheme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter

                Label {
                    font.pixelSize: AppTheme.fontSizeSmall
                    text: modelData.name
                }
                Label {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    font.pixelSize: AppTheme.fontSizeExtraSmall
                    color: AppTheme.secondaryColor
                    text: modelData.holder + " · " + modelData.text
                }
            }

            onClicked: page.play(modelData)
        }
    }

    Placeholder {
        enabled: view.count === 0
        text: "Niemand hält eine spielbare Ereigniskarte"
    }

    ScrollDecorator { flickableItem: view }
}
