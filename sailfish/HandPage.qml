/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 2.6
import Sailfish.Silica 1.0
import "."

// Every seat with its role, its city and its hand. During the discard phase the
// cards of the seat that is over the limit are tappable.
Page {
    id: page
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: page.width
            spacing: Theme.paddingSmall

            PageHeader {
                title: qsTr("Hände")
                description: engine.discardingSeat >= 0
                    ? qsTr("Abwerfen bis sieben Karten") : ""
            }

            Repeater {
                model: engine.players

                delegate: Column {
                    width: column.width
                    spacing: Theme.paddingSmall / 2

                    property var player: modelData
                    property bool discarding: engine.discardingSeat === modelData.seat

                    SectionHeader {
                        text: player.role + (player.atTurn ? qsTr(" — am Zug") : "")
                    }

                    Label {
                        x: Theme.horizontalPageMargin
                        width: parent.width - 2 * Theme.horizontalPageMargin
                        wrapMode: Text.WordWrap
                        font.pixelSize: Theme.fontSizeExtraSmall
                        color: Theme.secondaryColor
                        text: player.cityName + " · " + player.ability
                            + (player.stored.length > 0
                               ? qsTr(" · aufbewahrt: %1").arg(player.stored) : "")
                    }

                    Repeater {
                        model: player.hand

                        delegate: ListItem {
                            contentHeight: Theme.itemSizeExtraSmall
                            enabled: discarding

                            Rectangle {
                                x: Theme.horizontalPageMargin
                                anchors.verticalCenter: parent.verticalCenter
                                width: Theme.paddingSmall
                                height: parent.height * 0.6
                                color: modelData.colour >= 0 ? Style.colourOf(modelData.colour)
                                                             : Theme.highlightColor
                            }

                            Label {
                                x: Theme.horizontalPageMargin + Theme.paddingLarge
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData.label
                                color: discarding ? Theme.primaryColor : Theme.secondaryColor
                            }

                            onClicked: {
                                if (engine.discardCard(modelData.id) && engine.discardingSeat < 0)
                                    pageStack.pop()
                            }
                        }
                    }
                }
            }
        }
    }
}
