/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
#pragma once

// QtQuick 1.1 hat kein `Canvas`. Die 93 Verbindungslinien des Spielplans
// zeichnet auf dem N9 deshalb ein eigenes Element, das genauso aussieht wie
// der Canvas in sailfish/Board.qml — einschließlich des Sonderfalls über den
// Pazifik, wo die Linie sonst quer über die ganze Karte liefe.
//
// Ein QDeclarativeItem statt eines vorgerenderten Bildes, damit die Linien
// beim Zoomen scharf bleiben und nicht bei jeder Stufe neu erzeugt werden
// müssen.

#include <QColor>
#include <QDeclarativeItem>
#include <QVariantList>

class LinkItem : public QDeclarativeItem
{
    Q_OBJECT
    Q_PROPERTY(QVariantList links READ links WRITE setLinks NOTIFY linksChanged)
    Q_PROPERTY(QColor colour READ colour WRITE setColour NOTIFY colourChanged)

public:
    explicit LinkItem(QDeclarativeItem* parent = 0);

    QVariantList links() const { return m_links; }
    void setLinks(const QVariantList& links);

    QColor colour() const { return m_colour; }
    void setColour(const QColor& colour);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

signals:
    void linksChanged();
    void colourChanged();

private:
    QVariantList m_links;
    QColor m_colour;
};
