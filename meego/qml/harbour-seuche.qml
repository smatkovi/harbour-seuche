/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 1.1
import com.nokia.meego 1.0

// MeeGo-Harmattan-Ausgabe (Nokia N9). Der Aufbau folgt sailfish/, nur sind
// die Silica-Bausteine durch com.nokia.meego und die kleinen Nachbauten
// daneben ersetzt. `engine`, `Theme` und `Style` setzt meego/main.cpp als
// Kontexteigenschaften.
//
// Hier steht kein qsTr(): Qt 4.7 schiebt den Quelltext von qsTr() durch
// Latin-1, und die deutschen Texte sind voller Umlaute. Eine Übersetzung gibt
// es ohnehin nicht, also stehen die Zeichenketten unmittelbar da.
PageStackWindow {
    id: app

    showStatusBar: true
    showToolBar: true
    initialPage: MainPage { }

    Component.onCompleted: theme.inverted = true
}
