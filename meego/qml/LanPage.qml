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
        contentHeight: column.height + AppTheme.paddingLarge

        Column {
            id: column
            width: page.width
            spacing: AppTheme.paddingMedium

            PageHeader {
                title: "Netzwerk"
                description: engine.netStatus
            }

            // --- laufende Sitzung -----------------------------------------
            Column {
                width: parent.width
                spacing: AppTheme.paddingSmall
                visible: engine.netRole !== "local"

                SectionHeader { text: "Tisch" }

                Repeater {
                    model: engine.seatOwners()

                    delegate: Label {
                        x: AppTheme.horizontalPageMargin
                        width: column.width - 2 * AppTheme.horizontalPageMargin
                        wrapMode: Text.WordWrap
                        font.pixelSize: AppTheme.fontSizeSmall
                        color: modelData.mine ? AppTheme.highlightColor : AppTheme.secondaryColor
                        text: modelData.role + (modelData.mine ? " — dieses Gerät"
                                                               : " — anderes Gerät")
                    }
                }

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 4 * AppTheme.horizontalPageMargin
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
                spacing: AppTheme.paddingSmall
                visible: engine.netRole === "local"

                SectionHeader { text: "Spiel eröffnen" }

                Label {
                    x: AppTheme.horizontalPageMargin
                    font.pixelSize: AppTheme.fontSizeSmall
                    text: "Tischgröße: " + page.seatCount + " Sitze"
                }

                Slider {
                    x: AppTheme.horizontalPageMargin
                    width: parent.width - 2 * AppTheme.horizontalPageMargin
                    minimumValue: 2
                    maximumValue: 4
                    stepSize: 1
                    value: page.seatCount
                    onValueChanged: page.seatCount = Math.round(value)
                }

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 4 * AppTheme.horizontalPageMargin
                    text: "Schwierigkeit: " + page.difficultyNames[page.difficulty]
                    onClicked: difficultyDialog.open()
                }

                TextField {
                    id: tableName
                    x: AppTheme.horizontalPageMargin
                    width: parent.width - 2 * AppTheme.horizontalPageMargin
                    placeholderText: "Name des Tisches"
                }

                Label {
                    x: AppTheme.horizontalPageMargin
                    width: parent.width - 2 * AppTheme.horizontalPageMargin
                    wrapMode: Text.WordWrap
                    font.pixelSize: AppTheme.fontSizeExtraSmall
                    color: AppTheme.secondaryColor
                    text: "Der erste Sitz bleibt hier, jedes beitretende Gerät bekommt den "
                        + "nächsten. Sitze, die niemand übernimmt, werden hier gespielt.\n"
                        + "Dieses Gerät: " + engine.browser.localAddresses
                }

                Label {
                    x: AppTheme.horizontalPageMargin
                    width: parent.width - 2 * AppTheme.horizontalPageMargin
                    wrapMode: Text.WordWrap
                    font.pixelSize: AppTheme.fontSizeExtraSmall
                    color: AppTheme.secondaryColor
                    text: engine.bluetooth.available
                          ? "Bluetooth ist ebenfalls offen: " + (engine.bluetooth.localName !== ""
                                ? engine.bluetooth.localName : engine.bluetooth.localAddress)
                          : "Bluetooth ist aus, es kommt nur wer im selben Netz ist"
                }

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 4 * AppTheme.horizontalPageMargin
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
                spacing: AppTheme.paddingSmall
                visible: engine.netRole === "local"

                SectionHeader { text: "Beitreten" }

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 4 * AppTheme.horizontalPageMargin
                    text: engine.browser.searching ? "Suche läuft …" : "Im WLAN suchen"
                    enabled: !engine.browser.searching
                    onClicked: engine.browser.search()
                }

                Repeater {
                    model: engine.browser.hosts

                    delegate: ListItem {
                        contentHeight: AppTheme.itemSizeSmall

                        Column {
                            x: AppTheme.horizontalPageMargin
                            anchors.verticalCenter: parent.verticalCenter

                            Label {
                                font.pixelSize: AppTheme.fontSizeSmall
                                text: modelData.name
                            }
                            Label {
                                font.pixelSize: AppTheme.fontSizeExtraSmall
                                color: AppTheme.secondaryColor
                                text: modelData.address
                            }
                        }

                        onClicked: {
                            engine.joinGame(modelData.address)
                            page.leaving = true
                        }
                    }
                }

                SectionHeader { text: "Über Bluetooth beitreten" }

                Label {
                    x: AppTheme.horizontalPageMargin
                    width: parent.width - 2 * AppTheme.horizontalPageMargin
                    visible: engine.bluetooth.devices.length === 0
                    wrapMode: Text.WordWrap
                    font.pixelSize: AppTheme.fontSizeSmall
                    color: AppTheme.secondaryColor
                    text: engine.bluetooth.available
                          ? "Keine gekoppelten Geräte. Koppelt die Geräte einmal in den "
                            + "Systemeinstellungen, dann hier erneut suchen."
                          : "Bluetooth ist ausgeschaltet."
                }

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 4 * AppTheme.horizontalPageMargin
                    text: "Gekoppelte Geräte"
                    onClicked: engine.bluetooth.refresh()
                }

                Repeater {
                    model: engine.bluetooth.devices

                    delegate: ListItem {
                        contentHeight: AppTheme.itemSizeSmall

                        Column {
                            x: AppTheme.horizontalPageMargin
                            anchors.verticalCenter: parent.verticalCenter

                            Label {
                                font.pixelSize: AppTheme.fontSizeSmall
                                text: modelData.name
                            }
                            Label {
                                font.pixelSize: AppTheme.fontSizeExtraSmall
                                color: AppTheme.secondaryColor
                                text: modelData.address
                            }
                        }

                        onClicked: {
                            engine.joinBluetoothGame(modelData.address)
                            page.leaving = true
                        }
                    }
                }

                SectionHeader { text: "Adresse eingeben" }

                TextField {
                    id: manual
                    x: AppTheme.horizontalPageMargin
                    width: parent.width - 2 * AppTheme.horizontalPageMargin
                    placeholderText: "Adresse von Hand, 192.168.…"
                    inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                }

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 4 * AppTheme.horizontalPageMargin
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
