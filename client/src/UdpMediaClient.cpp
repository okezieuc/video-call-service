#include "UdpMediaClient.h"

#include "Protocol.h"

#include <QDebug>

UdpMediaClient::UdpMediaClient(QObject *parent)
    : QObject(parent), m_socket(new QUdpSocket(this)) {
  connect(m_socket, &QUdpSocket::readyRead, this,
          &UdpMediaClient::handleReadyRead);
}

void UdpMediaClient::configure(const QHostAddress &serverAddress,
                               quint16 serverPort, std::uint32_t clientId) {
  m_serverAddress = serverAddress;
  m_serverPort = serverPort;
  m_clientId = clientId;
  m_configured = serverPort != 0 && clientId != 0;
  m_reassembler.clear();

  if (m_configured && m_socket->state() != QAbstractSocket::BoundState) {
    if (!m_socket->bind(QHostAddress::AnyIPv4, 0)) {
      emit packetDropped(QString("failed to bind UDP socket: %1")
                             .arg(m_socket->errorString()));
    }
  }
}

void UdpMediaClient::sendRegisterEndpoint() {
  if (!m_configured) {
    emit packetDropped("UDP media client is not configured");
    return;
  }

  writePacket(Protocol::UdpPacketType::RegisterEndpoint, QByteArray());
}

void UdpMediaClient::setMediaEnabled(bool enabled) { m_mediaEnabled = enabled; }

std::uint64_t UdpMediaClient::receivedPacketCount() const {
  return m_receivedPacketCount;
}

void UdpMediaClient::sendVideoPacket(const QByteArray &encodedPacket) {
  if (!m_configured || !m_mediaEnabled || encodedPacket.isEmpty()) {
    return;
  }

  const auto frameId = m_nextFrameId++;
  const qsizetype maxPayloadSize = Protocol::MaxUdpPayloadSize;
  const auto fragmentCount = static_cast<std::uint16_t>(
      (encodedPacket.size() + maxPayloadSize - 1) / maxPayloadSize);

  for (std::uint16_t fragmentIndex = 0; fragmentIndex < fragmentCount;
       ++fragmentIndex) {
    const auto offset =
        static_cast<qsizetype>(fragmentIndex) * maxPayloadSize;
    const auto size = qMin(maxPayloadSize, encodedPacket.size() - offset);
    writePacket(Protocol::UdpPacketType::VideoPacket,
                encodedPacket.mid(offset, size), frameId, fragmentIndex,
                fragmentCount);
  }
}

void UdpMediaClient::handleReadyRead() {
  while (m_socket->hasPendingDatagrams()) {
    QByteArray datagram;
    datagram.resize(static_cast<qsizetype>(m_socket->pendingDatagramSize()));

    QHostAddress senderAddress;
    quint16 senderPort = 0;
    m_socket->readDatagram(datagram.data(), datagram.size(), &senderAddress,
                           &senderPort);

    Protocol::UdpPacket packet;
    if (!Protocol::decodeUdpPacket(datagram, packet)) {
      emit packetDropped("malformed UDP packet");
      continue;
    }

    if (packet.header.clientId == m_clientId) {
      emit packetDropped("ignored looped-back local packet");
      continue;
    }

    if (packet.header.packetType != Protocol::UdpPacketType::VideoPacket) {
      continue;
    }

    ++m_receivedPacketCount;
    qDebug() << "Received UDP video packet from client"
             << packet.header.clientId << "sequence"
             << packet.header.sequenceNumber << "payload"
             << packet.payload.size();
    emit remotePacketReceived(packet.header.clientId,
                              packet.header.sequenceNumber,
                              packet.payload.size());

    const auto reassemblyResult = m_reassembler.accept(packet);
    switch (reassemblyResult.status) {
    case UdpFragmentReassembler::Status::Complete:
      emit remoteVideoPacketReceived(packet.header.clientId,
                                     reassemblyResult.payload);
      break;
    case UdpFragmentReassembler::Status::Dropped:
      emit packetDropped(reassemblyResult.dropReason);
      break;
    case UdpFragmentReassembler::Status::Incomplete:
      break;
    }
  }
}

void UdpMediaClient::writePacket(std::uint8_t packetType,
                                 const QByteArray &payload,
                                 std::uint32_t frameId,
                                 std::uint16_t fragmentIndex,
                                 std::uint16_t fragmentCount) {
  if (!m_configured || payload.size() > Protocol::MaxUdpPayloadSize) {
    emit packetDropped("UDP payload is too large");
    return;
  }

  Protocol::UdpPacket packet;
  packet.header.packetType = packetType;
  packet.header.clientId = m_clientId;
  packet.header.sequenceNumber = m_nextSequenceNumber++;
  packet.header.timestampMs = Protocol::currentTimestampMs();
  packet.header.frameId = frameId;
  packet.header.fragmentIndex = fragmentIndex;
  packet.header.fragmentCount = fragmentCount;
  packet.payload = payload;

  const auto datagram = Protocol::encodeUdpPacket(packet);
  const auto bytesWritten =
      m_socket->writeDatagram(datagram, m_serverAddress, m_serverPort);
  if (bytesWritten != datagram.size()) {
    emit packetDropped("failed to write UDP datagram");
  }
}
