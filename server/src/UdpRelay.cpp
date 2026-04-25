#include "UdpRelay.h"

#include "Protocol.h"

#include <QDebug>

UdpRelay::UdpRelay(quint16 port, QObject *parent)
    : QObject(parent), m_socket(new QUdpSocket(this)), m_requestedPort(port) {
  connect(m_socket, &QUdpSocket::readyRead, this, &UdpRelay::handleReadyRead);
}

bool UdpRelay::start(const QHostAddress &address) {
  if (m_socket->state() == QAbstractSocket::BoundState) {
    return true;
  }

  const bool ok = m_socket->bind(address, m_requestedPort);
  if (ok) {
    qDebug() << "UDP relay listening on port" << m_socket->localPort();
  } else {
    qWarning() << "Failed to bind UDP relay on port" << m_requestedPort << ":"
               << m_socket->errorString();
  }
  return ok;
}

quint16 UdpRelay::port() const { return m_socket->localPort(); }

void UdpRelay::registerClient(std::uint32_t clientId) {
  if (clientId == 0 || m_clients.contains(clientId)) {
    return;
  }

  m_clients.insert(clientId, ClientEndpoint{});
}

void UdpRelay::unregisterClient(std::uint32_t clientId) {
  m_clients.remove(clientId);
}

bool UdpRelay::registerEndpointForClient(std::uint32_t clientId,
                                         const QHostAddress &senderAddress,
                                         quint16 senderPort) {
  if (!m_clients.contains(clientId)) {
    return false;
  }

  auto &client = m_clients[clientId];
  client.address = senderAddress;
  client.port = senderPort;
  client.registered = true;
  client.lastSeenMs = Protocol::currentTimestampMs();
  return true;
}

QVector<std::uint32_t>
UdpRelay::forwardingClientIds(std::uint32_t senderClientId) const {
  QVector<std::uint32_t> clientIds;
  for (auto it = m_clients.cbegin(); it != m_clients.cend(); ++it) {
    if (it.key() != senderClientId && it.value().registered) {
      clientIds.append(it.key());
    }
  }

  return clientIds;
}

void UdpRelay::handleReadyRead() {
  while (m_socket->hasPendingDatagrams()) {
    QByteArray datagram;
    datagram.resize(static_cast<qsizetype>(m_socket->pendingDatagramSize()));

    QHostAddress senderAddress;
    quint16 senderPort = 0;
    m_socket->readDatagram(datagram.data(), datagram.size(), &senderAddress,
                           &senderPort);
    handleDatagram(datagram, senderAddress, senderPort);
  }
}

bool UdpRelay::endpointMatches(const ClientEndpoint &client,
                               const QHostAddress &senderAddress,
                               quint16 senderPort) const {
  return client.registered && client.address == senderAddress &&
         client.port == senderPort;
}

void UdpRelay::handleDatagram(const QByteArray &datagram,
                              const QHostAddress &senderAddress,
                              quint16 senderPort) {
  Protocol::UdpPacket packet;
  if (!Protocol::decodeUdpPacket(datagram, packet)) {
    emit packetDropped("malformed UDP packet");
    return;
  }

  if (!m_clients.contains(packet.header.clientId)) {
    emit packetDropped("unknown client id");
    return;
  }

  if (packet.header.packetType == Protocol::UdpPacketType::RegisterEndpoint) {
    handleRegisterEndpoint(packet.header.clientId, senderAddress, senderPort);
    return;
  }

  auto &client = m_clients[packet.header.clientId];
  if (!endpointMatches(client, senderAddress, senderPort)) {
    ++client.malformedPackets;
    emit packetDropped("UDP endpoint mismatch");
    return;
  }

  client.lastSeenMs = Protocol::currentTimestampMs();
  ++client.packetsReceived;

  if (packet.header.packetType == Protocol::UdpPacketType::VideoPacket) {
    forwardVideoPacket(packet.header.clientId, datagram);
  }
}

void UdpRelay::handleRegisterEndpoint(std::uint32_t clientId,
                                      const QHostAddress &senderAddress,
                                      quint16 senderPort) {
  if (!registerEndpointForClient(clientId, senderAddress, senderPort)) {
    emit packetDropped("unknown client id");
    return;
  }

  qDebug() << "Registered UDP endpoint for client" << clientId << "at"
           << senderAddress << ":" << senderPort;
  emit endpointRegistered(clientId);
}

void UdpRelay::forwardVideoPacket(std::uint32_t senderClientId,
                                  const QByteArray &datagram) {
  std::uint32_t forwardedCount = 0;

  const auto clientIds = forwardingClientIds(senderClientId);
  for (const auto clientId : clientIds) {
    auto &client = m_clients[clientId];

    const auto bytesWritten =
        m_socket->writeDatagram(datagram, client.address, client.port);
    if (bytesWritten == datagram.size()) {
      ++forwardedCount;
      ++client.packetsForwarded;
    } else {
      emit packetDropped("failed to forward UDP packet");
    }
  }

  qDebug() << "Forwarded video packet from client" << senderClientId << "to"
           << forwardedCount << "clients";
  emit videoPacketForwarded(senderClientId, forwardedCount);
}
