/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 2.6
import Sailfish.Silica 1.0
import "."

// A plain chooser for cities, seats, infection cards or roles. `entries` is a
// list of maps; the caller gets back whichever number identifies the entry.
//
// The answer comes back as a signal, and callers connect to it on the page
// object that push() returns:
//
//     var picker = pageStack.push(Qt.resolvedUrl("PickPage.qml"), { ... })
//     picker.picked.connect(function (value) { ... })
//
// It used to be a `property var onPicked` filled in from push()'s property
// table instead. That table travels as a QVariantMap, a JavaScript function
// does not survive the trip, and the property stayed null -- so every chooser
// simply did nothing when tapped, which took out the event cards that need a
// target and the role picker with them.
Page {
    id: page
    allowedOrientations: Orientation.All

    property string title: ""
    property var entries: []

    signal picked(int value)

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

    SilicaListView {
        anchors.fill: parent
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
                        + (modelData.current === true ? qsTr(" — jetzt") : "")
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

        VerticalScrollDecorator { }
    }
}
