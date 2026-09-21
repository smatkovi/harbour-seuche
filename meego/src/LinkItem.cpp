/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "LinkItem.h"

#include <QPainter>
#include <QPen>
#include <QVariantMap>
#include <cmath>

LinkItem::LinkItem(QDeclarativeItem* parent)
    : QDeclarativeItem(parent)
    , m_colour(255, 255, 255, 56)
{
    setFlag(QGraphicsItem::ItemHasNoContents, false);
}

void LinkItem::setLinks(const QVariantList& links)
{
    if (m_links == links)
        return;
    m_links = links;
    emit linksChanged();
    update();
}

void LinkItem::setColour(const QColor& colour)
{
    if (m_colour == colour)
        return;
    m_colour = colour;
    emit colourChanged();
    update();
}

void LinkItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    const qreal w = width();
    const qreal h = height();
    if (w <= 0 || h <= 0 || m_links.isEmpty())
        return;

    QPen pen(m_colour);
    pen.setWidthF(qMax(qreal(1), w / 1000));
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing, true);

    for (int i = 0; i < m_links.size(); ++i) {
        const QVariantMap link = m_links.at(i).toMap();
        const qreal x1 = link.value(QString::fromLatin1("x1")).toReal();
        const qreal y1 = link.value(QString::fromLatin1("y1")).toReal();
        const qreal x2 = link.value(QString::fromLatin1("x2")).toReal();
        const qreal y2 = link.value(QString::fromLatin1("y2")).toReal();

        if (std::fabs(x1 - x2) > 0.4) {
            // Über den Pazifik: zwei Stummel über den Rand, sonst legt sich
            // die Linie quer über alles. Gleiche Regel wie in Board.qml.
            const bool firstIsWest = x1 < x2;
            const qreal wx = firstIsWest ? x1 : x2;
            const qreal wy = firstIsWest ? y1 : y2;
            const qreal ex = firstIsWest ? x2 : x1;
            const qreal ey = firstIsWest ? y2 : y1;
            painter->drawLine(QPointF(wx * w, wy * h), QPointF(0, wy * h));
            painter->drawLine(QPointF(ex * w, ey * h), QPointF(w, ey * h));
            continue;
        }
        painter->drawLine(QPointF(x1 * w, y1 * h), QPointF(x2 * w, y2 * h));
    }
}
