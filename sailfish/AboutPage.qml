/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 2.6
import Sailfish.Silica 1.0
import "."

Page {
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader { title: qsTr("Über Seuche") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Kooperatives Seuchen-Brettspiel für 2 bis 4 Personen an einem Gerät.\n\n"
                           + "Regelwerk, Spielplan und Datenmodell stehen in spec/ im Quelltext; "
                           + "der Regelkern ist reines C++ und wird von zwei Testprogrammen geprüft.\n\n"
                           + "Erweiterungsmodule (fünfte Seuche, virulenter Stamm, Labor, weltweite "
                           + "Panik, Bioterrorist) sind im Datenmodell vorgesehen, aber noch nicht "
                           + "gebaut.\n\nGPL-3.0-or-later")
            }
        }
    }
}
