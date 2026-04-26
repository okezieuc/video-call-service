#include "TcpServer.h"
#include "ConnectionHandler.h"
#include "UdpRelay.h"

#include <QDebug>

TcpServer::TcpServer(quint16 port, quint16 udpPort, QObject *parent)
    : QTcpServer(parent), m_port(port), m_udpRelay(new UdpRelay(udpPort, this)) {
  m_udpRelayRunning = m_udpRelay->start();
  if (!m_udpRelayRunning) {
    qWarning() << "Failed to start UDP relay on port" << udpPort;
  }

  connect(m_udpRelay, &UdpRelay::endpointRegistered, this,
          [this](std::uint32_t clientId) {
            handleUdpEndpointRegistered(clientId);
          });
}

bool TcpServer::isUdpRelayRunning() const { return m_udpRelayRunning; }

void TcpServer::incomingConnection(qintptr socketDescriptor) {
  auto *handler = new ConnectionHandler(socketDescriptor, this);

  connect(handler, &ConnectionHandler::joinRequested, this,
          [this](ConnectionHandler *handler) { handleJoinRequested(handler); });
  connect(handler, &ConnectionHandler::clientDisconnected, this,
          [this](ConnectionHandler *handler) {
            handleClientDisconnected(handler);
          });

  emit clientConnected(handler->peerAddress().toString(), handler->peerPort());
}

void TcpServer::handleJoinRequested(ConnectionHandler *handler) {
  if (m_clientsByHandler.contains(handler)) {
    handler->sendJoinAccepted(m_udpRelay->port());
    return;
  }

  const auto clientId = m_nextClientId++;
  handler->setClientId(clientId);
  m_clientsByHandler.insert(handler, clientId);
  m_handlersByClientId.insert(clientId, handler);
  m_udpRelay->registerClient(clientId);
  handler->sendJoinAccepted(m_udpRelay->port());
}

void TcpServer::handleClientDisconnected(ConnectionHandler *handler) {
  emit clientDisconnected(handler->peerAddress().toString(),
                          handler->peerPort());

  const auto clientId = m_clientsByHandler.take(handler);
  if (clientId != 0) {
    m_handlersByClientId.remove(clientId);
    m_udpRelay->unregisterClient(clientId);
  }

  handler->deleteLater();
}

void TcpServer::handleUdpEndpointRegistered(std::uint32_t clientId) {
  auto *handler = m_handlersByClientId.value(clientId, nullptr);
  if (!handler) {
    return;
  }

  handler->sendUdpRegistered();
}
