/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
import QtQuick 2.6
import Sailfish.Silica 1.0
import "."

ApplicationWindow {
    // main.cpp installs the one C++ instance as the root context property
    // `engine`; every page reads it from there.
    initialPage: Component { MainPage { } }
}
