/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 1.1
import com.nokia.meego 1.0

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
            spacing: Theme.paddingMedium

            PageHeader { title: "Über Seuche" }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                text: "Kooperatives Seuchen-Brettspiel für 2 bis 4 Personen an einem Gerät.\n\n"
                    + "Ausgabe für MeeGo Harmattan (Nokia N9). Regelkern, Spiellogik und "
                    + "Netzwerkspiel sind dieselben wie in der Sailfish-Fassung; nur die "
                    + "Oberfläche ist für QtQuick 1.1 eigens gebaut.\n\n"
                    + "Regelwerk, Spielplan und Datenmodell stehen in spec/ im Quelltext; "
                    + "der Regelkern ist reines C++ und wird von eigenen Testprogrammen "
                    + "geprüft, die auch tausende vollständige Partien durchspielen.\n\n"
                    + "Erweiterungsmodule (fünfte Seuche, virulenter Stamm, Labor, weltweite "
                    + "Panik, Bioterrorist) sind im Datenmodell vorgesehen, aber noch nicht "
                    + "gebaut.\n\nGPL-3.0-or-later"
            }
        }
    }

    ScrollDecorator { flickableItem: view }
}
