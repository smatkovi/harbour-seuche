/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 2.6
import Sailfish.Silica 1.0
import "."

// Rollenwahl. Jede Rolle gibt es nur einmal — eine vergebene steht gar nicht
// erst zur Wahl. Im Netzwerkspiel wählt jedes Gerät für seinen eigenen Sitz,
// und der Gastgeber prüft die Wahl noch einmal, bevor sie für alle gilt.
// Sobald die Partie läuft, stehen die Rollen fest.
Page {
    id: page
    objectName: "rolePage"
    allowedOrientations: Orientation.All

    // false when the page is opened from the board again
    property bool showStart: true

    SilicaListView {
        anchors.fill: parent
        model: engine.players

        header: PageHeader {
            title: qsTr("Rollen")
            description: engine.rolesLocked
                ? qsTr("Die Partie läuft — die Rollen stehen fest")
                : qsTr("Frei wählbar, jede nur einmal")
        }

        delegate: ListItem {
            id: seatItem
            contentHeight: Theme.itemSizeMedium

            property bool mine: engine.mySeats.indexOf(modelData.seat) >= 0
            enabled: !engine.rolesLocked && mine

            Rectangle {
                x: Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                width: Theme.fontSizeLarge
                height: width
                radius: width / 2
                color: Style.roleColourOf(modelData.roleId)
                border.width: 1
                border.color: Qt.rgba(0, 0, 0, 0.65)
            }

            Column {
                x: Theme.horizontalPageMargin + Theme.fontSizeLarge + Theme.paddingMedium
                width: parent.width - x - Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter

                Label {
                    text: qsTr("Sitz %1 — %2").arg(modelData.seat + 1).arg(modelData.role)
                    color: seatItem.enabled ? Theme.primaryColor : Theme.secondaryColor
                }

                Label {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeExtraSmall
                    color: Theme.secondaryColor
                    text: (engine.netRole === "local"
                           ? "" : (seatItem.mine ? qsTr("dieses Gerät · ") : qsTr("anderes Gerät · ")))
                          + modelData.ability
                }
            }

            onClicked: {
                var seat = modelData.seat
                var picker = pageStack.push(Qt.resolvedUrl("PickPage.qml"), {
                    title: qsTr("Rolle für Sitz %1").arg(seat + 1),
                    entries: engine.freeRoles(seat)
                })
                picker.picked.connect(function (roleId) {
                    engine.chooseRole(seat, roleId)
                    pageStack.pop()
                })
            }
        }

        footer: Column {
            width: parent.width
            spacing: Theme.paddingMedium
            topPadding: Theme.paddingLarge

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryColor
                visible: !engine.rolesLocked
                text: engine.netRole === "local"
                    ? qsTr("Nicht gewählte Sitze behalten ihre zufällig gezogene Rolle.")
                    : qsTr("Jedes Gerät wählt für seinen eigenen Sitz. Was ein anderes "
                           + "Gerät schon genommen hat, erscheint hier nicht mehr.")
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                visible: page.showStart
                text: qsTr("Zum Spielplan")
                onClicked: {
                    var board = pageStack.find(function (p) { return p.objectName === "boardPage" })
                    if (board)
                        pageStack.pop(board)
                    else
                        pageStack.push(Qt.resolvedUrl("BoardPage.qml"))
                }
            }
        }

        VerticalScrollDecorator { }
    }
}
