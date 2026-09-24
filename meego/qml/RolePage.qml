/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 1.1
import com.nokia.meego 1.0

// Rollenwahl. Jede Rolle gibt es nur einmal — eine vergebene steht gar nicht
// erst zur Wahl. Im Netzwerkspiel wählt jedes Gerät für seinen eigenen Sitz.
// Sobald die Partie läuft, stehen die Rollen fest.
Page {
    id: page
    objectName: "rolePage"
    orientationLock: PageOrientation.Automatic

    // false, wenn die Seite vom Spielplan aus geöffnet wird
    property bool showStart: true

    tools: ToolBarLayout {
        ToolIcon { iconId: "toolbar-back"; onClicked: pageStack.pop() }
    }

    ListView {
        id: view
        anchors.fill: parent
        clip: true
        model: engine.players

        header: PageHeader {
            title: "Rollen"
            description: engine.rolesLocked ? "Die Partie läuft — die Rollen stehen fest"
                                            : "Frei wählbar, jede nur einmal"
        }

        delegate: ListItem {
            id: seatItem
            contentHeight: AppTheme.itemSizeMedium

            property bool mine: engine.mySeats.indexOf(modelData.seat) >= 0
            enabled: !engine.rolesLocked && mine

            Rectangle {
                x: AppTheme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                width: AppTheme.fontSizeLarge
                height: width
                radius: width / 2
                color: Style.roleColourOf(modelData.roleId)
                border.width: 1
                border.color: Qt.rgba(0, 0, 0, 0.65)
            }

            Column {
                x: AppTheme.horizontalPageMargin + AppTheme.fontSizeLarge + AppTheme.paddingMedium
                width: parent.width - x - AppTheme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter

                Label {
                    font.pixelSize: AppTheme.fontSizeSmall
                    text: "Sitz " + (modelData.seat + 1) + " — " + modelData.role
                    color: seatItem.enabled ? AppTheme.primaryColor : AppTheme.secondaryColor
                }

                Label {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    font.pixelSize: AppTheme.fontSizeExtraSmall
                    color: AppTheme.secondaryColor
                    text: (engine.netRole === "local"
                           ? "" : (seatItem.mine ? "dieses Gerät · " : "anderes Gerät · "))
                          + modelData.ability
                }
            }

            onClicked: {
                var seat = modelData.seat
                var picker = pageStack.push(Qt.resolvedUrl("PickPage.qml"), {
                    title: "Rolle für Sitz " + (seat + 1),
                    entries: engine.freeRoles(seat)
                })
                picker.picked.connect(function (roleId) {
                    engine.chooseRole(seat, roleId)
                    pageStack.pop()
                })
            }
        }

        footer: Column {
            width: view.width
            spacing: AppTheme.paddingMedium

            Item { width: 1; height: AppTheme.paddingLarge }

            Label {
                x: AppTheme.horizontalPageMargin
                width: parent.width - 2 * AppTheme.horizontalPageMargin
                wrapMode: Text.WordWrap
                font.pixelSize: AppTheme.fontSizeExtraSmall
                color: AppTheme.secondaryColor
                visible: !engine.rolesLocked
                text: engine.netRole === "local"
                    ? "Nicht gewählte Sitze behalten ihre zufällig gezogene Rolle."
                    : "Jedes Gerät wählt für seinen eigenen Sitz. Was ein anderes Gerät "
                      + "schon genommen hat, erscheint hier nicht mehr."
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 4 * AppTheme.horizontalPageMargin
                visible: page.showStart
                text: "Zum Spielplan"
                onClicked: {
                    var board = pageStack.find(function (p) { return p.objectName === "boardPage" })
                    if (board)
                        pageStack.pop(board)
                    else
                        pageStack.push(Qt.resolvedUrl("BoardPage.qml"))
                }
            }
        }
    }

    ScrollDecorator { flickableItem: view }
}
