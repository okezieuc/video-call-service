#pragma once

#include <QTcpServer>

class TcpServer : public QTcpServer {
  Q_OBJECT

public:
  explicit TcpServer(quint16 port = 5555, QObject *parent = nullptr);

signals:
  void clientConnected(const QString &address, quint16 port);
  void clientDisconnected(const QString &address, quint16 port);

protected:
  void incomingConnection(qintptr socketDescriptor) override;

private:
  quint16 m_port;
};
