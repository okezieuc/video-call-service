#include "ConnectionHandler.h"

#include "SharedTypes.h"

#include <QDebug>

ConnectionHandler::ConnectionHandler(qintptr socketDescriptor, QObject *parent)
    : QTcpSocket(parent) {
  if (!setSocketDescriptor(socketDescriptor)) {
    qWarning() << "Failed to attach socket descriptor:" << errorString();
    deleteLater();
    return;
  }

  connect(this, &QTcpSocket::readyRead, this,
          &ConnectionHandler::handleReadyRead);
  connect(this, &QTcpSocket::disconnected, this,
          &ConnectionHandler::disconnected);

  qDebug() << "Client connected from" << peerAddress() << ":" << peerPort();
}

void ConnectionHandler::handleReadyRead() {
  m_receivedData.append(readAll());

  while (!m_receivedData.isEmpty()) {
    const auto type = static_cast<std::uint8_t>(m_receivedData.at(0));
    m_receivedData.remove(0, 1);

    switch (type) {
    case ControlMessage::Ping:
      sendPong();
      break;
    case ControlMessage::EndCall:
      qDebug() << "Received end call from" << peerAddress() << ":"
               << peerPort();
      break;
    case ControlMessage::CameraOff:
      qDebug() << "Received camera off from" << peerAddress() << ":"
               << peerPort();
      break;
    case ControlMessage::CameraOn:
      qDebug() << "Received camera on from" << peerAddress() << ":"
               << peerPort();
      break;
    case ControlMessage::Heartbeat:
      qDebug() << "Received heartbeat from" << peerAddress() << ":"
               << peerPort();
      break;
    default:
      qDebug() << "Unknown control message type:" << type;
      break;
    }
  }
}

void ConnectionHandler::sendPong() {
  sendMessage(ControlMessage::Pong);
  qDebug() << "Sent pong to" << peerAddress() << ":" << peerPort();
}

void ConnectionHandler::sendMessage(std::uint8_t type,
                                    const QByteArray &payload) {
  QByteArray message;
  message.append(static_cast<char>(type));
  message.append(payload);
  write(message);
  flush();
}
