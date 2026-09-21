/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 1.1
import com.nokia.meego 1.0

// Eine schlichte Auswahl für Städte, Sitze, Infektionskarten oder Rollen.
// `entries` ist eine Liste von Abbildungen; zurück kommt die Zahl, die den
// Eintrag benennt.
//
// Die Antwort kommt als Signal, und die Aufrufer verbinden sich damit an dem
// Seitenobjekt, das push() zurückgibt — genau wie in der Sailfish-Fassung:
//
//     var picker = pageStack.push(Qt.resolvedUrl("PickPage.qml"), { ... })
//     picker.picked.connect(function (value) { ... })
Page {
    id: page
    orientationLock: PageOrientation.Automatic

    property string title: ""
    property variant entries: []

    signal picked(int value)

    tools: ToolBarLayout {
        ToolIcon { iconId: "toolbar-back"; onClicked: pageStack.pop() }
    }

    function labelOf(entry) {
        return entry.name !== undefined ? entry.name : entry.role
    }

    function detailOf(entry) {
        if (entry.ability !== undefined)
            return entry.ability
        if (entry.cityName !== undefined)
            return entry.cityName
        return ""
    }

    function valueOf(entry) {
        if (entry.id !== undefined)
            return entry.id
        if (entry.roleId !== undefined)
            return entry.roleId
        return entry.seat
    }

    ListView {
        id: view
        anchors.fill: parent
        clip: true
        model: page.entries

        header: PageHeader { title: page.title }

        delegate: ListItem {
            contentHeight: page.detailOf(modelData) === "" ? Theme.itemSizeSmall
                                                           : Theme.itemSizeMedium

            Column {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter

                Label {
                    text: page.labelOf(modelData)
                        + (modelData.current === true ? " — jetzt" : "")
                    font.pixelSize: Theme.fontSizeSmall
                    color: modelData.current === true ? Theme.highlightColor : Theme.primaryColor
                }

                Label {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    visible: text.length > 0
                    font.pixelSize: Theme.fontSizeExtraSmall
                    color: Theme.secondaryColor
                    text: page.detailOf(modelData)
                }
            }

            onClicked: page.picked(page.valueOf(modelData))
        }
    }

    ScrollDecorator { flickableItem: view }
}
