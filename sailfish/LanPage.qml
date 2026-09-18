/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 2.6
import Sailfish.Silica 1.0
import "."

// Netzwerkspiel: ein Gerät eröffnet, die anderen treten bei. Es gibt nichts zu
// verbergen — der ganze Zustand außer der Reihenfolge der beiden Decks ist
// offen —, also spiegelt der Gastgeber schlicht alles und bleibt der einzige,
// der die Partie verändert.
Page {
    id: page
    objectName: "lanPage"
    allowedOrientations: Orientation.All

    property int seatCount: 3
    property int difficulty: 1
    property bool leaving: false

    // Everyone picks a role first; what another device has taken is gone from
    // the list before it can be picked twice.
    function openRoles() {
        var roles = pageStack.find(function (p) { return p.objectName === "rolePage" })
        if (!roles)
            pageStack.push(Qt.resolvedUrl("RolePage.qml"))
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: page.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: qsTr("Netzwerk")
                description: engine.netStatus
            }

            // --- running session ------------------------------------------
            Column {
                width: parent.width
                spacing: Theme.paddingSmall
                visible: engine.netRole !== "local"

                SectionHeader { text: qsTr("Tisch") }

                Repeater {
                    model: engine.seatOwners()

                    delegate: Label {
                        x: Theme.horizontalPageMargin
                        width: parent.width - 2 * Theme.horizontalPageMargin
                        font.pixelSize: Theme.fontSizeSmall
                        color: modelData.mine ? Theme.highlightColor : Theme.secondaryColor
                        text: modelData.role + (modelData.mine ? qsTr(" — dieses Gerät")
                                                               : qsTr(" — anderes Gerät"))
                    }
                }

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: engine.netRole === "host" ? qsTr("Tisch schließen") : qsTr("Verbindung trennen")
                    onClicked: {
                        engine.leaveNetwork()
                        pageStack.pop()
                    }
                }
            }

            // --- host -----------------------------------------------------
            Column {
                width: parent.width
                spacing: Theme.paddingSmall
                visible: engine.netRole === "local"

                SectionHeader { text: qsTr("Spiel eröffnen") }

                Slider {
                    width: parent.width
                    minimumValue: 2
                    maximumValue: 4
                    stepSize: 1
                    value: page.seatCount
                    valueText: qsTr("%1 Sitze").arg(value)
                    label: qsTr("Tischgröße")
                    onValueChanged: page.seatCount = value
                }

                ComboBox {
                    width: parent.width
                    label: qsTr("Schwierigkeit")
                    currentIndex: page.difficulty
                    menu: ContextMenu {
                        MenuItem { text: qsTr("Einführung") }
                        MenuItem { text: qsTr("Normal") }
                        MenuItem { text: qsTr("Heroisch") }
                    }
                    onCurrentIndexChanged: page.difficulty = currentIndex
                }

                TextField {
                    id: tableName
                    width: parent.width
                    label: qsTr("Name des Tisches")
                    placeholderText: qsTr("Seuche")
                    EnterKey.iconSource: "image://theme/icon-m-enter-close"
                    EnterKey.onClicked: focus = false
                }

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeExtraSmall
                    color: Theme.secondaryColor
                    text: qsTr("Der erste Sitz bleibt hier, jedes beitretende Gerät bekommt den "
                               + "nächsten. Sitze, die niemand übernimmt, werden hier gespielt.\n"
                               + "Dieses Gerät: %1").arg(engine.browser.localAddresses)
                }

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Tisch eröffnen")
                    onClicked: {
                        if (engine.hostGame(page.seatCount, page.difficulty, tableName.text))
                            page.openRoles()
                    }
                }
            }

            // --- join -----------------------------------------------------
            Column {
                width: parent.width
                spacing: Theme.paddingSmall
                visible: engine.netRole === "local"

                SectionHeader { text: qsTr("Beitreten") }

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: engine.browser.searching ? qsTr("Suche läuft …") : qsTr("Im WLAN suchen")
                    enabled: !engine.browser.searching
                    onClicked: engine.browser.search()
                }

                Repeater {
                    model: engine.browser.hosts

                    delegate: ListItem {
                        contentHeight: Theme.itemSizeSmall

                        Column {
                            x: Theme.horizontalPageMargin
                            anchors.verticalCenter: parent.verticalCenter

                            Label { text: modelData.name }
                            Label {
                                font.pixelSize: Theme.fontSizeExtraSmall
                                color: Theme.secondaryColor
                                text: modelData.address
                            }
                        }

                        onClicked: {
                            engine.joinGame(modelData.address)
                            page.leaving = true
                        }
                    }
                }

                TextField {
                    id: manual
                    width: parent.width
                    label: qsTr("Adresse von Hand")
                    placeholderText: qsTr("192.168.…")
                    inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                    EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                    EnterKey.onClicked: {
                        engine.joinGame(text)
                        page.leaving = true
                    }
                }
            }
        }
    }

    // A guest lands on the board as soon as the host's first state arrives.
    Connections {
        target: engine
        onChanged: {
            if (page.leaving && engine.running && engine.netRole === "guest") {
                page.leaving = false
                page.openRoles()
            }
        }
    }
}
