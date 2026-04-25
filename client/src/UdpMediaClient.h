#pragma once

#include <cstdint>

#include <QByteArray>
#include <QHostAddress>
#include <QObject>
#include <QString>
#include <QUdpSocket>

class UdpMediaClient : public QObject {
  Q_OBJECT

public:
  explicit UdpMediaClient(QObject *parent = nullptr);

  void configure(const QHostAddress &serverAddress, quint16 serverPort,
                 std::uint32_t clientId);
  void sendRegisterEndpoint();
  void setMediaEnabled(bool enabled);
  std::uint64_t receivedPacketCount() const;

public slots:
  void sendVideoPacket(const QByteArray &encodedPacket);

signals:
  void remotePacketReceived(std::uint32_t senderClientId,
                            std::uint32_t sequenceNumber,
                            qsizetype payloadSize);
  void packetDropped(const QString &reason);

private slots:
  void handleReadyRead();

private:
  void writePacket(std::uint8_t packetType, const QByteArray &payload,
                   std::uint32_t frameId = 0,
                   std::uint16_t fragmentIndex = 0,
                   std::uint16_t fragmentCount = 1);

  QUdpSocket *m_socket = nullptr;
  QHostAddress m_serverAddress;
  quint16 m_serverPort = 0;
  std::uint32_t m_clientId = 0;
  std::uint32_t m_nextSequenceNumber = 1;
  std::uint32_t m_nextFrameId = 1;
  std::uint64_t m_receivedPacketCount = 0;
  bool m_configured = false;
  bool m_mediaEnabled = false;
};
