#include "ConnectionHandler.h"

#include "Protocol.h"
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
  connect(this, &QTcpSocket::disconnected, this, [this]() {
    emit clientDisconnected(this);
  });

  qDebug() << "Client connected from" << peerAddress() << ":" << peerPort();
}

std::uint32_t ConnectionHandler::clientId() const { return m_clientId; }

void ConnectionHandler::setClientId(std::uint32_t clientId) {
  m_clientId = clientId;
}

void ConnectionHandler::sendJoinAccepted(std::uint16_t udpPort) {
  Protocol::JoinAcceptedPayload payload{m_clientId, udpPort};
  sendMessage(ControlMessage::JoinAccepted,
              Protocol::encodeJoinAcceptedPayload(payload));
  qDebug() << "Accepted client" << m_clientId << "with UDP port" << udpPort;
}

void ConnectionHandler::sendUdpRegistered() {
  sendMessage(ControlMessage::UdpRegistered);
  qDebug() << "Confirmed UDP registration for client" << m_clientId;
}

void ConnectionHandler::handleReadyRead() {
  m_receivedData.append(readAll());

  while (true) {
    Protocol::TcpMessage message;
    const auto result = Protocol::takeTcpMessage(m_receivedData, message);
    if (result == Protocol::DecodeResult::Incomplete) {
      return;
    }

    if (result == Protocol::DecodeResult::Invalid) {
      qDebug() << "Invalid TCP control frame from" << peerAddress() << ":"
               << peerPort();
      disconnectFromHost();
      return;
    }

    switch (message.type) {
    case ControlMessage::Ping:
      sendPong();
      break;
    case ControlMessage::JoinCall:
      emit joinRequested(this);
      break;
    case ControlMessage::EndCall:
    case ControlMessage::LeaveCall:
      qDebug() << "Received end call from" << peerAddress() << ":"
               << peerPort();
      disconnectFromHost();
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
      qDebug() << "Unknown control message type:" << message.type;
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
  write(Protocol::encodeTcpMessage(type, payload));
  flush();
}
