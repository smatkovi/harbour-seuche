/*
    Copyright (C) 2026 smatkovi

    This file is part of harbour-seuche.

    harbour-seuche is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    SPDX-License-Identifier: GPL-3.0-or-later
*/
#pragma once

#include "core/Rules.h"
#include "core/Setup.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

#include <random>
#include <vector>

// The QML facade of the rule core. QML never sees a rule: it asks for the legal
// actions, gets them with a finished German label, and sends back the index of
// the one that was tapped. Everything that could go wrong stays on this side.
//
// All seats are played on this device (co-operative hot seat). The seat count is
// chosen when the game starts; the network mode will later mark seats as remote.
class SeucheEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running NOTIFY changed)
    Q_PROPERTY(bool over READ over NOTIFY changed)
    Q_PROPERTY(QString phase READ phase NOTIFY changed)
    Q_PROPERTY(QString phaseText READ phaseText NOTIFY changed)
    Q_PROPERTY(QString outcomeText READ outcomeText NOTIFY changed)
    Q_PROPERTY(int seats READ seats NOTIFY changed)
    Q_PROPERTY(int atTurn READ atTurn NOTIFY changed)
    Q_PROPERTY(int actionsLeft READ actionsLeft NOTIFY changed)
    Q_PROPERTY(int drawsLeft READ drawsLeft NOTIFY changed)
    Q_PROPERTY(int infectionsLeft READ infectionsLeft NOTIFY changed)
    Q_PROPERTY(int infectionRate READ infectionRate NOTIFY changed)
    Q_PROPERTY(int outbreaks READ outbreaks NOTIFY changed)
    Q_PROPERTY(int stations READ stations NOTIFY changed)
    Q_PROPERTY(int playerDeck READ playerDeck NOTIFY changed)
    Q_PROPERTY(int infectionDeck READ infectionDeck NOTIFY changed)
    Q_PROPERTY(int discardingSeat READ discardingSeat NOTIFY changed)
    Q_PROPERTY(QVariantList cities READ cities NOTIFY changed)
    Q_PROPERTY(QVariantList links READ links CONSTANT)
    Q_PROPERTY(QVariantList players READ players NOTIFY changed)
    Q_PROPERTY(QVariantList cures READ cures NOTIFY changed)
    Q_PROPERTY(QStringList journal READ journal NOTIFY changed)

public:
    explicit SeucheEngine(QObject *parent = nullptr);

    bool running() const { return running_; }
    bool over() const { return running_ && game_.over(); }
    QString phase() const;
    QString phaseText() const;
    QString outcomeText() const;
    int seats() const { return running_ ? int(game_.players.size()) : 0; }
    int atTurn() const { return running_ ? game_.atTurn : 0; }
    int actionsLeft() const { return running_ ? game_.actionsLeft : 0; }
    int drawsLeft() const { return running_ ? game_.drawsLeft : 0; }
    int infectionsLeft() const { return running_ ? game_.infectionsLeft : 0; }
    int infectionRate() const { return running_ ? game_.infectionRate() : 0; }
    int outbreaks() const { return running_ ? game_.outbreaks : 0; }
    int stations() const { return running_ ? game_.stationCount() : 0; }
    int playerDeck() const { return running_ ? int(game_.playerDeck.size()) : 0; }
    int infectionDeck() const { return running_ ? int(game_.infectionDeck.size()) : 0; }
    int discardingSeat() const;
    QVariantList cities() const;
    QVariantList links() const;
    QVariantList players() const;
    QVariantList cures() const;
    QStringList journal() const { return journal_; }

    // seats 2..4, difficulty 0 = introduction, 1 = normal, 2 = heroic
    Q_INVOKABLE void newGame(int seats, int difficulty);

    // Legal actions, each as { index, label, city, kind }. `city` is where the
    // action happens on the board, so the board page can offer it on a tap.
    Q_INVOKABLE QVariantList actionsForCity(int cityId) const;
    Q_INVOKABLE QVariantList allActions() const;
    Q_INVOKABLE bool run(int index);

    Q_INVOKABLE void drawCard();
    Q_INVOKABLE void infectCity();
    Q_INVOKABLE bool discardCard(int cardId);

    // Event cards anyone may play right now, as { seat, event, name, text,
    // needs }. `needs` says what the page has to ask for: "", "city", "pawn",
    // "infection" or "forecast".
    Q_INVOKABLE QVariantList eventCards() const;
    Q_INVOKABLE bool playEvent(int seat, int event, int cityId, int pawn);
    // The infection discard pile, newest last — the "Zähe Bevölkerung" event
    // picks one of these.
    Q_INVOKABLE QVariantList infectionDiscard() const;
    Q_INVOKABLE QVariantList forecastCards() const;
    Q_INVOKABLE bool playForecast(int seat, const QVariantList &order);

    Q_INVOKABLE QString cityName(int cityId) const;
    Q_INVOKABLE QString cardLabel(int cardId) const;

signals:
    void changed();

private:
    void refresh();                       // recompute the legal actions, emit changed
    void note(const QString &line);
    QString label(const seuche::Action &action) const;
    QString seatName(int seat) const;

    seuche::Game game_;
    std::vector<seuche::Action> legal_;
    std::mt19937 rng_;
    QStringList journal_;
    bool running_ = false;
};
