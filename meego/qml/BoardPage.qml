/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 1.1
import com.nokia.meego 1.0

// Die Spielseite: Stand, Karte, Handkarten und der eine Knopf, auf den die
// laufende Phase wartet. Die Karte selbst steckt in Board.qml und gibt es
// unter „Karte groß“ noch einmal formatfüllend.
Page {
    id: page
    objectName: "boardPage"
    orientationLock: PageOrientation.Automatic

    function toMainPage() {
        var main = pageStack.find(function (p) { return p.objectName === "mainPage" })
        if (main)
            pageStack.pop(main)
        else
            pageStack.push(Qt.resolvedUrl("MainPage.qml"))
    }

    tools: ToolBarLayout {
        ToolIcon { iconId: "toolbar-back"; onClicked: pageStack.pop() }
        ToolIcon { iconId: "toolbar-view-menu"; onClicked: boardMenu.open() }
    }

    Menu {
        id: boardMenu
        MenuLayout {
            MenuItem {
                text: "Rückgängig: " + engine.undoText
                visible: engine.canUndo && engine.mayAct
                onClicked: engine.undo()
            }
            MenuItem {
                text: "Neue Partie"
                onClicked: page.toMainPage()
            }
            MenuItem {
                text: "Protokoll"
                onClicked: pageStack.push(Qt.resolvedUrl("LogPage.qml"))
            }
            MenuItem {
                text: "Hände und Rollen"
                onClicked: pageStack.push(Qt.resolvedUrl("HandPage.qml"))
            }
            MenuItem {
                text: "Rollen wählen"
                visible: !engine.rolesLocked
                onClicked: pageStack.push(Qt.resolvedUrl("RolePage.qml"), { showStart: false })
            }
            MenuItem {
                text: "Ereigniskarte spielen"
                onClicked: pageStack.push(Qt.resolvedUrl("EventPage.qml"))
            }
            MenuItem {
                text: "Netzwerk"
                onClicked: pageStack.push(Qt.resolvedUrl("LanPage.qml"))
            }
        }
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
            spacing: Theme.paddingSmall

            PageHeader {
                title: engine.phaseText
                description: {
                    if (engine.players.length === 0)
                        return ""
                    var line = engine.players[engine.atTurn].role + " — "
                             + engine.players[engine.atTurn].cityName
                    if (engine.netRole !== "local")
                        line += engine.mayAct ? " · du bist dran" : " · anderes Gerät ist dran"
                    return line
                }
            }

            // --- Stand ----------------------------------------------------
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
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryColor
                text: "Rate " + engine.infectionRate + " · Ausbrüche " + engine.outbreaks
                    + "/8 · Stationen " + engine.stations + "/6 · Deck " + engine.playerDeck
                    + " · Infektionen " + engine.infectionDeck
            }

            // --- Karte ----------------------------------------------------
            Board {
                id: boardView
                width: column.width
                height: page.height * 0.42
                actionsEnabled: engine.mayAct
                onCityClicked: pageStack.push(Qt.resolvedUrl("ActionPage.qml"), { cityId: cityId })
                Component.onCompleted: centreOnTurn()
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 4 * Theme.horizontalPageMargin
                text: "Karte groß"
                onClicked: pageStack.push(Qt.resolvedUrl("MapPage.qml"))
            }

            // --- Hand der Person am Zug -----------------------------------
            //
            // Die Karten sind antippbar, denn am Tisch denkt man in Karten und
            // nicht in Städten auf dem Plan: eine Stadtkarte öffnet die
            // Aktionen für ihre Stadt (dort steht dann auch der Direktflug),
            // eine Ereigniskarte die Ereignisseite.
            Flow {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                spacing: Theme.paddingSmall / 2

                Repeater {
                    model: engine.players.length > 0 ? engine.players[engine.atTurn].hand : []

                    delegate: Rectangle {
                        id: card
                        property variant entry: modelData
                        height: Theme.itemSizeExtraSmall * 0.5
                        width: cardLabel.width + Theme.paddingMedium
                        radius: 4
                        color: entry.colour >= 0 ? Style.colourOf(entry.colour)
                                                 : Theme.secondaryHighlightColor
                        opacity: cardArea.pressed ? 0.6 : 1.0

                        Label {
                            id: cardLabel
                            anchors.centerIn: parent
                            font.pixelSize: Theme.fontSizeExtraSmall
                            color: "#101010"
                            text: card.entry.label
                        }

                        MouseArea {
                            id: cardArea
                            anchors.fill: parent
                            onClicked: {
                                if (card.entry.event)
                                    pageStack.push(Qt.resolvedUrl("EventPage.qml"))
                                else
                                    pageStack.push(Qt.resolvedUrl("ActionPage.qml"),
                                                   { cityId: card.entry.id })
                            }
                        }
                    }
                }
            }

            // --- worauf die Phase wartet ----------------------------------
            //
            // Nach der vierten Aktion endet die Aktionsphase von selbst. Bevor
            // die erste Karte aufgedeckt wird — und damit das Zurücknehmen
            // endgültig vorbei ist — wird hier gefragt, ob der Zug wirklich
            // vorbei sein soll.
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                visible: engine.awaitingTurnEnd
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
                text: "Zug beenden? Rückgängig geht noch."
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: Theme.paddingMedium

                Button {
                    visible: engine.phase === "actions"
                    enabled: engine.mayAct
                    text: "Aktionen"
                    onClicked: pageStack.push(Qt.resolvedUrl("ActionPage.qml"), { cityId: -1 })
                }
                Button {
                    visible: engine.phase === "draw"
                    enabled: engine.mayAct
                    text: engine.awaitingTurnEnd ? "Ja, Karten ziehen"
                                                 : "Karte ziehen (" + engine.drawsLeft + ")"
                    onClicked: engine.drawCard()
                }
                Button {
                    visible: engine.canUndo && engine.mayAct
                    text: engine.awaitingTurnEnd ? "Nein, zurück" : "Rückgängig"
                    onClicked: engine.undo()
                }
                Button {
                    visible: engine.phase === "discard"
                    enabled: engine.mayAct
                    text: "Abwerfen"
                    onClicked: pageStack.push(Qt.resolvedUrl("HandPage.qml"))
                }
                Button {
                    visible: engine.phase === "infect"
                    enabled: engine.mayAct
                    text: "Infizieren (" + engine.infectionsLeft + ")"
                    onClicked: engine.infectCity()
                }
                Button {
                    visible: engine.over
                    text: "Neue Partie"
                    onClicked: page.toMainPage()
                }
            }

            // die letzten Zeilen des Protokolls, damit der Tisch mitkommt
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

    ScrollDecorator { flickableItem: view }
}
