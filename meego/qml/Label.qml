/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 1.1

// Silica bringt `Label` mit, com.nokia.meego nicht. Damit die Seiten hier
// Zeile für Zeile mit denen unter sailfish/ vergleichbar bleiben, gibt es das
// hier nach: ein Text mit den Vorgaben aus AppTheme.
Text {
    color: AppTheme.primaryColor
    font.pixelSize: AppTheme.fontSizeMedium
    font.family: "Nokia Pure Text"
    textFormat: Text.PlainText
    elide: Text.ElideNone
}
