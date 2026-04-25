#pragma once

#include <cstdint>

#include <QTcpSocket>

class ConnectionHandler : public QTcpSocket {
  Q_OBJECT

public:
  explicit ConnectionHandler(qintptr socketDescriptor,
                             QObject *parent = nullptr);

signals:
  void connected();
  void disconnected();

private slots:
  void handleReadyRead();

private:
  void sendPong();
  void sendMessage(std::uint8_t type, const QByteArray &payload = QByteArray());

  QByteArray m_receivedData;
};
