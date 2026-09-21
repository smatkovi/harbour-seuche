/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 1.1
import com.nokia.meego 1.0

// Jeder Sitz mit seiner Rolle, seiner Stadt und seiner Hand. Während der
// Abwurfphase lassen sich die Karten des Sitzes antippen, der über dem Limit
// liegt.
Page {
    id: page
    orientationLock: PageOrientation.Automatic

    tools: ToolBarLayout {
        ToolIcon { iconId: "toolbar-back"; onClicked: pageStack.pop() }
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
                title: "Hände"
                description: engine.discardingSeat >= 0 ? "Abwerfen bis sieben Karten" : ""
            }

            Repeater {
                model: engine.players

                delegate: Column {
                    id: seatBlock
                    width: column.width
                    spacing: Theme.paddingSmall / 2

                    property variant player: modelData
                    property bool discarding: engine.discardingSeat === modelData.seat

                    Item {
                        width: parent.width
                        height: heading.height

                        SectionHeader {
                            id: heading
                            text: seatBlock.player.role
                                + (seatBlock.player.atTurn ? " — am Zug" : "")
                        }

                        // dieselbe Farbe wie die Figur auf dem Spielplan
                        Rectangle {
                            anchors.left: parent.left
                            anchors.leftMargin: Theme.horizontalPageMargin
                            anchors.verticalCenter: heading.verticalCenter
                            width: Theme.fontSizeMedium
                            height: width
                            radius: width / 2
                            color: Style.roleColourOf(seatBlock.player.roleId)
                            border.width: 1
                            border.color: Qt.rgba(0, 0, 0, 0.65)
                        }
                    }

                    Label {
                        x: Theme.horizontalPageMargin
                        width: parent.width - 2 * Theme.horizontalPageMargin
                        wrapMode: Text.WordWrap
                        font.pixelSize: Theme.fontSizeExtraSmall
                        color: Theme.secondaryColor
                        text: seatBlock.player.cityName + " · " + seatBlock.player.ability
                            + (seatBlock.player.stored.length > 0
                               ? " · aufbewahrt: " + seatBlock.player.stored : "")
                    }

                    Repeater {
                        model: seatBlock.player.hand

                        delegate: ListItem {
                            contentHeight: Theme.itemSizeExtraSmall
                            enabled: seatBlock.discarding

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
                                font.pixelSize: Theme.fontSizeSmall
                                text: modelData.label
                                color: seatBlock.discarding ? Theme.primaryColor
                                                            : Theme.secondaryColor
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

    ScrollDecorator { flickableItem: view }
}
