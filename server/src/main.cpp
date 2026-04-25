#include "TcpServer.h"

#include <QCoreApplication>
#include <QDebug>
#include <QHostAddress>

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);

  constexpr quint16 port = 5555;
  TcpServer server(port);
  if (!server.listen(QHostAddress::Any, port)) {
    qCritical() << "Failed to start server on port" << port << ":"
                << server.errorString();
    return 1;
  }

  qDebug() << "Server listening on port" << port;
  return app.exec();
}
