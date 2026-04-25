#include "TcpServer.h"
#include "ConnectionHandler.h"

TcpServer::TcpServer(quint16 port, QObject *parent)
    : QTcpServer(parent), m_port(port) {}

void TcpServer::incomingConnection(qintptr socketDescriptor) {
  auto *handler = new ConnectionHandler(socketDescriptor, this);

  connect(handler, &ConnectionHandler::disconnected, this, [this, handler]() {
    emit clientDisconnected(handler->peerAddress().toString(),
                            handler->peerPort());
    handler->deleteLater();
  });

  emit clientConnected(handler->peerAddress().toString(), handler->peerPort());
}
