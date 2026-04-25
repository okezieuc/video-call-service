#pragma once

#include <QTcpServer>
#include <QHash>

#include <cstdint>

class ConnectionHandler;
class UdpRelay;

class TcpServer : public QTcpServer {
  Q_OBJECT

public:
  explicit TcpServer(quint16 port = 5555, quint16 udpPort = 5556,
                     QObject *parent = nullptr);
  bool isUdpRelayRunning() const;

signals:
  void clientConnected(const QString &address, quint16 port);
  void clientDisconnected(const QString &address, quint16 port);

protected:
  void incomingConnection(qintptr socketDescriptor) override;

private:
  void handleJoinRequested(ConnectionHandler *handler);
  void handleClientDisconnected(ConnectionHandler *handler);
  void handleUdpEndpointRegistered(std::uint32_t clientId);

  quint16 m_port;
  UdpRelay *m_udpRelay = nullptr;
  bool m_udpRelayRunning = false;
  std::uint32_t m_nextClientId = 1;
  QHash<ConnectionHandler *, std::uint32_t> m_clientsByHandler;
  QHash<std::uint32_t, ConnectionHandler *> m_handlersByClientId;
};
