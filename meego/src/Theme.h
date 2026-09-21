/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
#pragma once

// Zwei Zusammenhalter für die N9-Oberfläche.
//
// Silica bringt `Theme` (Maße und Farben) und harbour-seuche bringt `Style`
// (Seuchen- und Rollenfarben) als QML-Singleton mit. QtQuick 1.1 kennt kein
// `pragma Singleton`, und com.nokia.meego hat kein `Theme`. Beide werden
// deshalb hier in C++ nachgebildet und in main.cpp als Kontexteigenschaften
// gesetzt — dann bleiben die Seiten unter meego/qml Zeile für Zeile mit denen
// unter sailfish/ vergleichbar, statt sich in Zahlen zu unterscheiden.
//
// Die Maße sind die des N9: 480x854, und die Schriftgrößen die der
// MeeGo-Komponenten.

#include <QColor>
#include <QObject>
#include <QVariantList>

class MeegoTheme : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int paddingSmall READ paddingSmall CONSTANT)
    Q_PROPERTY(int paddingMedium READ paddingMedium CONSTANT)
    Q_PROPERTY(int paddingLarge READ paddingLarge CONSTANT)
    Q_PROPERTY(int horizontalPageMargin READ horizontalPageMargin CONSTANT)
    Q_PROPERTY(int itemSizeExtraSmall READ itemSizeExtraSmall CONSTANT)
    Q_PROPERTY(int itemSizeSmall READ itemSizeSmall CONSTANT)
    Q_PROPERTY(int itemSizeMedium READ itemSizeMedium CONSTANT)
    Q_PROPERTY(int fontSizeTiny READ fontSizeTiny CONSTANT)
    Q_PROPERTY(int fontSizeExtraSmall READ fontSizeExtraSmall CONSTANT)
    Q_PROPERTY(int fontSizeSmall READ fontSizeSmall CONSTANT)
    Q_PROPERTY(int fontSizeMedium READ fontSizeMedium CONSTANT)
    Q_PROPERTY(int fontSizeLarge READ fontSizeLarge CONSTANT)
    Q_PROPERTY(QColor primaryColor READ primaryColor CONSTANT)
    Q_PROPERTY(QColor secondaryColor READ secondaryColor CONSTANT)
    Q_PROPERTY(QColor highlightColor READ highlightColor CONSTANT)
    Q_PROPERTY(QColor secondaryHighlightColor READ secondaryHighlightColor CONSTANT)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor CONSTANT)

public:
    explicit MeegoTheme(QObject* parent = 0) : QObject(parent) {}

    int paddingSmall() const { return 6; }
    int paddingMedium() const { return 12; }
    int paddingLarge() const { return 24; }
    int horizontalPageMargin() const { return 16; }
    int itemSizeExtraSmall() const { return 64; }
    int itemSizeSmall() const { return 72; }
    int itemSizeMedium() const { return 88; }
    int fontSizeTiny() const { return 14; }
    int fontSizeExtraSmall() const { return 18; }
    int fontSizeSmall() const { return 22; }
    int fontSizeMedium() const { return 26; }
    int fontSizeLarge() const { return 32; }
    QColor primaryColor() const { return QColor(0xff, 0xff, 0xff); }
    QColor secondaryColor() const { return QColor(0x9a, 0x9a, 0xa4); }
    QColor highlightColor() const { return QColor(0x6c, 0xb8, 0xff); }
    QColor secondaryHighlightColor() const { return QColor(0x3a, 0x46, 0x55); }
    QColor backgroundColor() const { return QColor(0x0b, 0x0f, 0x14); }
};

// Dieselben Werte wie sailfish/Style.qml, Zahl für Zahl.
class MeegoStyle : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QColor station READ station CONSTANT)
    Q_PROPERTY(QColor pawn READ pawn CONSTANT)
    Q_PROPERTY(QColor link READ link CONSTANT)
    Q_PROPERTY(QColor sea READ sea CONSTANT)
    Q_PROPERTY(QVariantList diseaseName READ diseaseName CONSTANT)

public:
    explicit MeegoStyle(QObject* parent = 0) : QObject(parent) {}

    QColor station() const { return QColor(0xf4, 0xf4, 0xf4); }
    QColor pawn() const { return QColor(0xff, 0xff, 0xff); }
    QColor link() const { return QColor(255, 255, 255, 56); }
    QColor sea() const { return QColor(0x10, 0x18, 0x20); }

    QVariantList diseaseName() const
    {
        QVariantList list;
        list << QString::fromUtf8("Blau") << QString::fromUtf8("Gelb")
             << QString::fromUtf8("Schwarz") << QString::fromUtf8("Rot")
             << QString::fromUtf8("Violett");
        return list;
    }

    Q_INVOKABLE QColor colourOf(int index) const
    {
        static const char* const colours[] = {"#4c8fd4", "#ddb43a", "#8a8a96", "#d4544c", "#a05bd0"};
        if (index < 0 || index > 4)
            return QColor(0x80, 0x80, 0x80);
        return QColor(QString::fromLatin1(colours[index]));
    }

    Q_INVOKABLE QColor roleColourOf(int index) const
    {
        static const char* const colours[] = {"#f07f2a", "#9c6b3f", "#f2f2f2", "#e052a8",
                                              "#7ed957", "#2f8f4f", "#3fd0c9"};
        if (index < 0 || index > 6)
            return pawn();
        return QColor(QString::fromLatin1(colours[index]));
    }
};
