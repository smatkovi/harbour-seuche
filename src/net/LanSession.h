#pragma once

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTcpServer>
#include <QTimer>

#include "BtLink.h"
#include <QVariantList>
#include <QVariantMap>

class QTcpSocket;
class QUdpSocket;

// Transport for matches between devices, taken over from harbour-snapszer and
// given its own ports, RFCOMM channel and discovery words so the two games
// never answer each other's probes. One device hosts (TCP server plus a UDP
// responder so other devices can find it, and an RFCOMM channel for guests
// that come over Bluetooth), the others join over whichever they like. Messages are compact JSON objects, one per line. The
// session knows nothing about the game rules. A host accepts up to `maxPeers`
// guests; a guest has exactly one peer, the host, with id 0.
class LanSession : public QObject
{
    Q_OBJECT

public:
    enum Role { None = 0, Host = 1, Guest = 2 };

    static const quint16 GamePort = 45475;
    static const quint16 DiscoveryPort = 45476;

    explicit LanSession(QObject* parent = nullptr);
    ~LanSession() override;

    Role role() const { return m_role; }
    bool peerConnected() const { return !m_peers.isEmpty(); }
    int peerCount() const { return m_peers.size(); }

    // `players` is the table size shown to searching devices, `maxPeers` the
    // number of guests accepted.
    // Bluetooth is opened alongside the LAN whenever the device has an
    // adapter; a table is only refused when neither transport works.
    bool startHosting(const QString& hostName, int players, int maxPeers, QString* error);
    void setAcceptingGuests(bool accepting);
    void joinHost(const QString& address);
    // Joins a host over Bluetooth. `address` is a device address, not an IP.
    void joinBluetooth(const QString& address);
    bool bluetoothHosting() const;
    // Why the RFCOMM channel is not open, when hosting started without it.
    QString bluetoothError() const { return m_btError; }
    void stop();
    void send(const QVariantMap& message);
    void sendTo(int peer, const QVariantMap& message);
    void dropPeer(int peer);

    // IPv4 addresses for the local network, and global IPv6 addresses under
    // which the device may be reachable from the internet.
    static QStringList localAddresses();
    static QStringList internetAddresses();
    // Trims spaces and the brackets people type around IPv6 addresses.
    static QString normalizeAddress(const QString& address);

signals:
    void peerConnectedChanged();
    void peerJoined(int peer);
    void peerLost(int peer);
    void messageReceived(int peer, const QVariantMap& message);
    void connectionFailed(const QString& reason);

private:
    struct Peer {
        int id = -1;
        // A QTcpSocket or an RfcommSocket: both carry the same lines.
        QIODevice* socket = nullptr;
        QByteArray buffer;
        qint64 lastSeen = 0;
    };

    Peer* addPeer(QIODevice* socket);
    Peer* findPeer(int id);
    void removePeer(int id, bool notify);
    void readPeer(int id);
    void checkIdlePeers();
    // Which peer a socket belongs to. The per peer slots below recover it from
    // sender(), where the Qt 5 code captured it in a lambda.
    int peerIdOf(QObject* socket) const;
    static void writeLine(QIODevice* socket, const QVariantMap& message);

// Qt 4 has no pointer-to-member connect(), so everything a signal reaches must
// be a real slot named in a SIGNAL()/SLOT() string. Declaring them as slots
// costs Qt 5 nothing and keeps one source for both.
private slots:
    void acceptConnections();
    void answerDiscovery();
    void onPingTimeout();
    void onConnectTimeout();
    void onGuestConnected();
    void onGuestSocketError();
    void onPeerReadyRead();
    void onPeerDisconnected();
    void onBtConnection(RfcommSocket* socket);
    void onBtConnected();
    void onBtFailed(const QString& reason);

private:

    Role m_role = None;
    QString m_hostName;
    int m_players = 2;
    int m_maxPeers = 1;
    bool m_accepting = true;
    int m_nextPeerId = 0;
    QTcpServer m_server;
    QList<Peer> m_peers;
    QUdpSocket* m_responder = nullptr;
    QTcpSocket* m_pendingSocket = nullptr;
    RfcommServer* m_btServer = nullptr;
    RfcommSocket* m_pendingBt = nullptr;
    QString m_btError;
    QTimer m_pingTimer;
    QTimer m_connectTimer;
};

// Finds hosted games in the local network by UDP broadcast, or asks a single
// address directly.
class LanBrowser : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList hosts READ hosts NOTIFY hostsChanged)
    Q_PROPERTY(QString localAddresses READ localAddresses NOTIFY hostsChanged)
    Q_PROPERTY(QString internetAddresses READ internetAddresses NOTIFY hostsChanged)
    Q_PROPERTY(bool searching READ searching NOTIFY searchingChanged)

public:
    explicit LanBrowser(QObject* parent = nullptr);
    ~LanBrowser() override;

    QVariantList hosts() const { return m_hosts; }
    QString localAddresses() const { return LanSession::localAddresses().join(QStringLiteral(", ")); }
    QString internetAddresses() const { return LanSession::internetAddresses().join(QStringLiteral("\n")); }
    bool searching() const { return m_searching; }

    Q_INVOKABLE void search();
    Q_INVOKABLE void copyToClipboard(const QString& text);

signals:
    void hostsChanged();
    void searchingChanged();

private slots:
    void sendProbes();
    void readReplies();
    void finish();

private:
    bool ensureSocket();

    QUdpSocket* m_socket = nullptr;
    QTimer m_timer;
    QTimer m_finishTimer;
    int m_probesLeft = 0;
    bool m_searching = false;
    bool m_lockHeld = false;
    QVariantList m_hosts;
};
