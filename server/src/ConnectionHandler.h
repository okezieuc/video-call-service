#pragma once

#include <cstdint>

#include <QTcpSocket>

class ConnectionHandler : public QTcpSocket {
  Q_OBJECT

public:
  explicit ConnectionHandler(qintptr socketDescriptor,
                             QObject *parent = nullptr);
  std::uint32_t clientId() const;
  void setClientId(std::uint32_t clientId);
  void sendJoinAccepted(std::uint16_t udpPort);
  void sendUdpRegistered();

signals:
  void joinRequested(ConnectionHandler *handler);
  void clientDisconnected(ConnectionHandler *handler);

private slots:
  void handleReadyRead();

private:
  void sendPong();
  void sendMessage(std::uint8_t type, const QByteArray &payload = QByteArray());

  QByteArray m_receivedData;
  std::uint32_t m_clientId = 0;
};
