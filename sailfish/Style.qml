pragma Singleton
/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 2.6
import Sailfish.Silica 1.0
import "."



// The four diseases and the pieces, in one place. Violet is the fifth disease of
// the mutation module and only shows up once that is implemented.
QtObject {
    readonly property var disease: ["#4c8fd4", "#ddb43a", "#8a8a96", "#d4544c", "#a05bd0"]
    readonly property var diseaseName: ["Blau", "Gelb", "Schwarz", "Rot", "Violett"]
    readonly property color station: "#f4f4f4"
    readonly property color pawn: "#ffffff"

    // Eine feste Farbe je Rolle, wie im Brettspiel: die Figur gehört der Rolle,
    // nicht dem Sitz, damit am Tisch „die grüne" eindeutig die Quarantäne ist.
    // Bewusst weit weg von den vier Seuchenfarben oben — Figuren stehen direkt
    // neben den Würfeln und dürfen nicht mit ihnen verschwimmen.
    readonly property var roleColour: [
        "#f07f2a",   // Arzt — orange
        "#9c6b3f",   // Forscherin — braun
        "#f2f2f2",   // Wissenschaftlerin — weiß
        "#e052a8",   // Logistiker — magenta
        "#7ed957",   // Betriebsexpertin — hellgrün
        "#2f8f4f",   // Quarantäne — dunkelgrün
        "#3fd0c9"    // Planer — türkis
    ]
    readonly property color link: Qt.rgba(1, 1, 1, 0.22)
    readonly property color sea: "#101820"

    function colourOf(index) {
        return index >= 0 && index < disease.length ? disease[index] : "#808080"
    }

    function roleColourOf(index) {
        return index >= 0 && index < roleColour.length ? roleColour[index] : pawn
    }
}
