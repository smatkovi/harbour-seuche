/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 1.1
import com.nokia.meego 1.0

// Netzwerkspiel: ein Gerät eröffnet, die anderen treten bei. Es gibt nichts zu
// verbergen — der ganze Zustand außer der Reihenfolge der beiden Decks ist
// offen —, also spiegelt der Gastgeber schlicht alles und bleibt der einzige,
// der die Partie verändert.
//
// Qt 4 sockets are IPv4 only, so the N9 hosts over IPv4; finding and joining
// works the same in both directions.
Page {
    id: page
    objectName: "lanPage"
    orientationLock: PageOrientation.Automatic

    property int seatCount: 3
    property int difficulty: 1
    property bool leaving: false

    property variant difficultyNames: ["Einführung", "Normal", "Heroisch"]

    tools: ToolBarLayout {
        ToolIcon { iconId: "toolbar-back"; onClicked: pageStack.pop() }
    }

    // Jeder wählt zuerst eine Rolle; was ein anderes Gerät genommen hat, ist
    // aus der Liste verschwunden, bevor es zweimal gewählt werden kann.
    function openRoles() {
        var roles = pageStack.find(function (p) { return p.objectName === "rolePage" })
        if (!roles)
            pageStack.push(Qt.resolvedUrl("RolePage.qml"))
    }

    SelectionDialog {
        id: difficultyDialog
        titleText: "Schwierigkeit"
        selectedIndex: page.difficulty
        model: ListModel {
            ListElement { name: "Einführung" }
            ListElement { name: "Normal" }
            ListElement { name: "Heroisch" }
        }
        onAccepted: page.difficulty = difficultyDialog.selectedIndex
    }

    Flickable {
        id: view
        anchors.fill: parent
        clip: true
        contentWidth: width
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: page.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: "Netzwerk"
                description: engine.netStatus
            }

            // --- laufende Sitzung -----------------------------------------
            Column {
                width: parent.width
                spacing: Theme.paddingSmall
                visible: engine.netRole !== "local"

                SectionHeader { text: "Tisch" }

                Repeater {
                    model: engine.seatOwners()

                    delegate: Label {
                        x: Theme.horizontalPageMargin
                        width: column.width - 2 * Theme.horizontalPageMargin
                        wrapMode: Text.WordWrap
                        font.pixelSize: Theme.fontSizeSmall
                        color: modelData.mine ? Theme.highlightColor : Theme.secondaryColor
                        text: modelData.role + (modelData.mine ? " — dieses Gerät"
                                                               : " — anderes Gerät")
                    }
                }

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 4 * Theme.horizontalPageMargin
                    text: engine.netRole === "host" ? "Tisch schließen" : "Verbindung trennen"
                    onClicked: {
                        engine.leaveNetwork()
                        pageStack.pop()
                    }
                }
            }

            // --- eröffnen --------------------------------------------------
            Column {
                width: parent.width
                spacing: Theme.paddingSmall
                visible: engine.netRole === "local"

                SectionHeader { text: "Spiel eröffnen" }

                Label {
                    x: Theme.horizontalPageMargin
                    font.pixelSize: Theme.fontSizeSmall
                    text: "Tischgröße: " + page.seatCount + " Sitze"
                }

                Slider {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    minimumValue: 2
                    maximumValue: 4
                    stepSize: 1
                    value: page.seatCount
                    onValueChanged: page.seatCount = Math.round(value)
                }

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 4 * Theme.horizontalPageMargin
                    text: "Schwierigkeit: " + page.difficultyNames[page.difficulty]
                    onClicked: difficultyDialog.open()
                }

                TextField {
                    id: tableName
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    placeholderText: "Name des Tisches"
                }

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeExtraSmall
                    color: Theme.secondaryColor
                    text: "Der erste Sitz bleibt hier, jedes beitretende Gerät bekommt den "
                        + "nächsten. Sitze, die niemand übernimmt, werden hier gespielt.\n"
                        + "Dieses Gerät: " + engine.browser.localAddresses
                }

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 4 * Theme.horizontalPageMargin
                    text: "Tisch eröffnen"
                    onClicked: {
                        if (engine.hostGame(page.seatCount, page.difficulty, tableName.text))
                            page.openRoles()
                    }
                }
            }

            // --- beitreten -------------------------------------------------
            Column {
                width: parent.width
                spacing: Theme.paddingSmall
                visible: engine.netRole === "local"

                SectionHeader { text: "Beitreten" }

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 4 * Theme.horizontalPageMargin
                    text: engine.browser.searching ? "Suche läuft …" : "Im WLAN suchen"
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

                            Label {
                                font.pixelSize: Theme.fontSizeSmall
                                text: modelData.name
                            }
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
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    placeholderText: "Adresse von Hand, 192.168.…"
                    inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                }

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 4 * Theme.horizontalPageMargin
                    text: "Dieser Adresse beitreten"
                    enabled: manual.text.length > 0
                    onClicked: {
                        engine.joinGame(manual.text)
                        page.leaving = true
                    }
                }
            }
        }
    }

    ScrollDecorator { flickableItem: view }

    // Ein Gast landet auf dem Spielplan, sobald der erste Zustand des
    // Gastgebers ankommt.
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
