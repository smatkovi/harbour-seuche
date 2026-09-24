/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 1.1
import com.nokia.meego 1.0

// Was geschehen ist, das Neueste zuletzt.
Page {
    orientationLock: PageOrientation.Automatic

    tools: ToolBarLayout {
        ToolIcon { iconId: "toolbar-back"; onClicked: pageStack.pop() }
    }

    ListView {
        id: view
        anchors.fill: parent
        clip: true
        header: PageHeader { title: "Protokoll" }
        model: engine.journal

        delegate: Label {
            x: AppTheme.horizontalPageMargin
            width: view.width - 2 * AppTheme.horizontalPageMargin
            wrapMode: Text.WordWrap
            font.pixelSize: AppTheme.fontSizeSmall
            color: AppTheme.primaryColor
            text: modelData
        }
    }

    ScrollDecorator { flickableItem: view }
}
