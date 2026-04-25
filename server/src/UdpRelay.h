#pragma once

#include <cstdint>

#include <QHash>
#include <QHostAddress>
#include <QObject>
#include <QUdpSocket>
#include <QVector>

class UdpRelay : public QObject {
  Q_OBJECT

public:
  explicit UdpRelay(quint16 port = 5556, QObject *parent = nullptr);

  bool start(const QHostAddress &address = QHostAddress::AnyIPv4);
  quint16 port() const;
  void registerClient(std::uint32_t clientId);
  void unregisterClient(std::uint32_t clientId);
  bool registerEndpointForClient(std::uint32_t clientId,
                                 const QHostAddress &senderAddress,
                                 quint16 senderPort);
  QVector<std::uint32_t> forwardingClientIds(std::uint32_t senderClientId) const;

signals:
  void endpointRegistered(std::uint32_t clientId);
  void videoPacketForwarded(std::uint32_t senderClientId, std::uint32_t count);
  void packetDropped(const QString &reason);

private slots:
  void handleReadyRead();

private:
  struct ClientEndpoint {
    QHostAddress address;
    quint16 port = 0;
    bool registered = false;
    std::uint64_t lastSeenMs = 0;
    std::uint64_t packetsReceived = 0;
    std::uint64_t packetsForwarded = 0;
    std::uint64_t malformedPackets = 0;
  };

  bool endpointMatches(const ClientEndpoint &client,
                       const QHostAddress &senderAddress,
                       quint16 senderPort) const;
  void handleDatagram(const QByteArray &datagram,
                      const QHostAddress &senderAddress, quint16 senderPort);
  void handleRegisterEndpoint(std::uint32_t clientId,
                              const QHostAddress &senderAddress,
                              quint16 senderPort);
  void forwardVideoPacket(std::uint32_t senderClientId,
                          const QByteArray &datagram);

  QUdpSocket *m_socket = nullptr;
  quint16 m_requestedPort = 0;
  QHash<std::uint32_t, ClientEndpoint> m_clients;
};
