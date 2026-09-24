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
#include "net/LanSession.h"

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
// Without a network every seat is played on this device (co-operative hot seat).
// In a LAN game the host keeps the one real Game and is the only device that
// ever changes it: guests mirror the state, work out the legal actions locally
// for their own display, and send the chosen action over the wire.
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

    // network
    Q_PROPERTY(QString netRole READ netRole NOTIFY netChanged)
    Q_PROPERTY(QString netStatus READ netStatus NOTIFY netChanged)
    Q_PROPERTY(int peerCount READ peerCount NOTIFY netChanged)
    Q_PROPERTY(QVariantList mySeats READ mySeats NOTIFY netChanged)
    Q_PROPERTY(bool mayAct READ mayAct NOTIFY changed)
    // Roles stay changeable until the first thing happens in the game.
    Q_PROPERTY(bool rolesLocked READ rolesLocked NOTIFY changed)
    // Undo reaches back over everything that happened this turn while nothing
    // has been turned face up yet.
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY changed)
    Q_PROPERTY(QString undoText READ undoText NOTIFY changed)
    // True at the one moment the player should be asked whether the turn is
    // really over: the fourth action is spent, no card has been drawn yet.
    Q_PROPERTY(bool awaitingTurnEnd READ awaitingTurnEnd NOTIFY changed)
    Q_PROPERTY(LanBrowser *browser READ browser CONSTANT)
    // Eine angefangene Partie liegt auf der Platte und ist beim Start wieder
    // da. Die Startseite braucht dafuer nichts Eigenes: `running` ist dann
    // schon wahr, und ihr Knopf "Laufende Partie fortsetzen" steht da.

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

    bool canUndo() const;
    QString undoText() const;
    bool awaitingTurnEnd() const;
    // Takes back the last action, event or discard of this turn.
    Q_INVOKABLE bool undo();

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

    // The roles still free, plus the one this seat holds — what the role page
    // offers. A role another seat has taken never shows up, on any device.
    Q_INVOKABLE QVariantList freeRoles(int seat) const;
    Q_INVOKABLE bool chooseRole(int seat, int role);
    bool rolesLocked() const { return rolesLocked_; }

    Q_INVOKABLE QString cityName(int cityId) const;
    Q_INVOKABLE QString cardLabel(int cardId) const;

    // --- network ---
    QString netRole() const;
    QString netStatus() const { return netStatus_; }
    int peerCount() const { return session_.peerCount(); }
    QVariantList mySeats() const;
    // True when the seat that has to act right now belongs to this device.
    bool mayAct() const;
    LanBrowser *browser() { return &browser_; }

    // Starts a game and offers it on the network. `name` is what searching
    // devices see.
    Q_INVOKABLE bool hostGame(int seats, int difficulty, const QString &name);
    Q_INVOKABLE void joinGame(const QString &address);
    Q_INVOKABLE void leaveNetwork();
    // Die gespeicherte Partie wegwerfen. Fuer den Fall, dass jemand lieber neu
    // anfaengt, ohne erst eine Partie zu Ende zu spielen.
    Q_INVOKABLE void discardSavedGame();
    // Which seat a device holds, for the seat list on the LAN page.
    Q_INVOKABLE QVariantList seatOwners() const;

signals:
    void changed();
    void netChanged();

// Qt 4 (the Nokia N9 edition) connects by signature, so everything a signal
// reaches has to be a declared slot.
private slots:
    void handleMessage(int peer, const QVariantMap &message);
    void assignSeat(int peer);
    void releaseSeats(int peer);
    void onConnectionFailed(const QString &reason);

public slots:
    // Den Stand wegschreiben. Laeuft nach jeder Aenderung mit; der Anschluss an
    // aboutToQuit ist nur der Nachschlag fuer den geordneten Abgang.
    //
    // Ein echter Schlitz und kein Q_INVOKABLE: die MeeGo-Fassung haengt ihn
    // unter Qt 4 mit der Zeichenketten-Schreibweise an, und SLOT() findet nur
    // Schlitze.
    void saveGame();

private:
    void refresh();                       // recompute the legal actions, emit changed
    // --- die angefangene Partie ---------------------------------------------
    //
    // Gespeichert wird nur eine Partie, die diesem Geraet allein gehoert. Eine
    // Netzpartie nicht: der Gastgeber koennte zwar seinen Stand sichern, aber
    // wer an welchem Sitz sitzt, haengt an Verbindungen, die es nach dem
    // Neustart nicht mehr gibt -- eine halb wiederhergestellte Netzpartie waere
    // schlimmer als keine. Ein Gast hat ohnehin nur ein Spiegelbild mit
    // verdeckten Stapeln, und das darf nie ueber einen echten Stand.
    QString savePath() const;
    bool loadGame();
    void note(const QString &line);
    QString label(const seuche::Action &action) const;
    QString seatName(int seat) const;

    // The four ways the state changes. On a host these run here; a guest sends
    // the same thing over the wire and waits for the new state.
    bool applyAction(const seuche::Action &action);
    void applyDraw();
    void applyInfect();
    bool applyDiscardCard(int seat, int cardId);
    bool applyEventPlay(const seuche::EventPlay &play);
    bool applyRole(int seat, int role);   // no ownership check: the host's own path
    bool ownsSeat(int seat) const;

    // --- undo ---
    //
    // A whole Game is a few kilobytes of plain vectors and arrays, so taking a
    // copy before every move is cheaper and far safer than working out how to
    // reverse each of the twelve action kinds. The random generator travels
    // with it, so a redo after an undo plays out exactly as before.
    //
    // The stack only ever holds the current turn, and only while the game has
    // shown nobody anything: the first drawn card ends it. Taking back a move
    // after a card has been turned would be looking into the future, and this
    // is a co-operative game where that is simply cheating.
    struct Snapshot {
        seuche::Game game;
        std::mt19937 rng;
        QStringList journal;
        QString what;                     // what this would take back
    };
    bool pushUndo(const QString &what);   // false when nothing was stored
    void clearUndo();

    // --- network helpers ---
    bool isHost() const { return session_.role() == LanSession::Host; }
    bool isGuest() const { return session_.role() == LanSession::Guest; }
    int ownerOfSeat(int seat) const;          // peer id, -1 for this device
    int seatToAct() const;                    // whose turn it is to do something
    void sendState();                         // host: full state to every guest
    void setStatus(const QString &text);

    seuche::Game game_;
    std::vector<seuche::Action> legal_;
    std::mt19937 rng_;
    QStringList journal_;
    bool running_ = false;
    bool rolesLocked_ = false;
    // Diese Partie gehoert diesem Geraet allein und darf gesichert werden.
    bool local_ = false;

    LanSession session_;
    LanBrowser browser_;
    QString netStatus_;
    std::vector<int> seatOwner_;   // per seat: peer id, -1 = this device
    std::vector<int> mySeats_;     // guest: the seats this device was given
    std::vector<Snapshot> undo_;
};
