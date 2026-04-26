#include "MainWindow.h"
#include "FrameHandler.h"
#include "Protocol.h"
#include "SharedTypes.h"
#include "UdpMediaClient.h"
#include "VideoPreview.h"

#include <QApplication>
#include <QCamera>
#include <QCameraDevice>
#include <QDebug>
#include <QHostAddress>
#include <QImageCapture>
#include <QLabel>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QPainter>
#include <QPermissions>
#include <QPushButton>
#include <QTcpSocket>
#include <QVBoxLayout>
#include <QVideoSink>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  central = new QWidget(this);
  layout = new QVBoxLayout(central);
  joinCallButton = new QPushButton("Join Call", central);
  statusLabel = new QLabel("Disconnected", central);
  videoPreviewArea = new VideoPreview();
  remoteFeedsLabel = new QLabel("Remote participants", central);
  remoteFeedsLayout = new QVBoxLayout();
  layout->addWidget(joinCallButton);
  layout->addWidget(statusLabel);
  layout->addWidget(new QLabel("Local preview", central));
  layout->addWidget(videoPreviewArea);
  layout->addWidget(remoteFeedsLabel);
  layout->addLayout(remoteFeedsLayout);
  setCentralWidget(central);
  setWindowTitle("Video Call Client");
  resize(640, 520);

  videoSink = new QVideoSink;
  videoFrameHandler = new FrameHandler;
  // videoFrameHandler->enableSingleFrameDevMode();
  connect(videoSink, &QVideoSink::videoFrameChanged, videoFrameHandler,
          &FrameHandler::receiveFrame);
  connect(videoFrameHandler, &FrameHandler::newFrameAvailable, videoPreviewArea,
          &VideoPreview::updateNextFrame);

  camera = new QCamera;
  captureSession.setCamera(camera);
  captureSession.setVideoOutput(videoSink);

  connect(joinCallButton, &QPushButton::clicked, this,
          &MainWindow::onJoinCallClicked);
}

void MainWindow::updateCameraStatus() {
  if (videoInputCount() == 0) {
    cameraStatus.deviceHasCamera = Tristate::False;
    return;
  }

  cameraStatus.deviceHasCamera = Tristate::True;
  switch (checkCameraPermissionStatus()) {
  case Qt::PermissionStatus::Undetermined:
    requestCameraPermission();
    cameraStatus.isPermissionGranted = Tristate::Unknown;
    break;
  case Qt::PermissionStatus::Denied:
    qInfo("This app does not have the permission to access the camera.");
    cameraStatus.isPermissionGranted = Tristate::False;
    break;
  case Qt::PermissionStatus::Granted:
    cameraStatus.isPermissionGranted = Tristate::True;
    break;
  }

  // TODO: Move the logic for starting the camera to a more well thought out
  // place
  if (cameraStatus.isPermissionGranted == Tristate::True) {
    startCamera();
  }
}

Qt::PermissionStatus MainWindow::checkCameraPermissionStatus() const {
  return qApp->checkPermission(cameraPermission);
}

void MainWindow::requestCameraPermission() {
  qApp->requestPermission(cameraPermission, this,
                          &MainWindow::updateCameraStatus);
}

void MainWindow::startCamera() { camera->start(); }

int MainWindow::videoInputCount() const {
  return QMediaDevices::videoInputs().count();
}

void MainWindow::onJoinCallClicked() {
  if (!controlSocket) {
    controlSocket = new QTcpSocket(this);
    connect(controlSocket, &QTcpSocket::connected, this,
            &MainWindow::onConnected);
    connect(controlSocket, &QTcpSocket::disconnected, this,
            &MainWindow::onDisconnected);
    connect(controlSocket, &QTcpSocket::readyRead, this,
            &MainWindow::onReadyRead);
  }

  if (controlSocket->state() == QAbstractSocket::ConnectedState) {
    onConnected();
    return;
  }

  joinCallButton->setText("Connecting...");
  updateConnectionStatus("Connecting");
  controlSocket->connectToHost("127.0.0.1", 5555);
}

void MainWindow::onConnected() {
  qDebug() << "Connected to server; joining call";

  sendControlMessage(ControlMessage::Ping);
  sendControlMessage(ControlMessage::JoinCall);

  joinCallButton->setText("Joining...");
  updateConnectionStatus("Joining call");
}

void MainWindow::onDisconnected() {
  qDebug() << "Disconnected from server";
  clientId = 0;
  controlBuffer.clear();
  remotePacketCount = 0;
  remoteVideoPacketCount = 0;
  clearRemoteParticipants();
  if (udpMediaClient) {
    udpMediaClient->setMediaEnabled(false);
  }
  joinCallButton->setText("Join Call");
  updateConnectionStatus("Disconnected");
}

void MainWindow::onReadyRead() {
  controlBuffer.append(controlSocket->readAll());

  while (true) {
    Protocol::TcpMessage message;
    const auto result = Protocol::takeTcpMessage(controlBuffer, message);
    if (result == Protocol::DecodeResult::Incomplete) {
      return;
    }

    if (result == Protocol::DecodeResult::Invalid) {
      qDebug() << "Invalid control frame from server";
      controlSocket->disconnectFromHost();
      return;
    }

    handleControlMessage(message);
  }
}

void MainWindow::onRemotePacketReceived(std::uint32_t senderClientId,
                                        std::uint32_t sequenceNumber,
                                        qsizetype payloadSize) {
  ++remotePacketCount;
  updateConnectionStatus(
      QString("UDP registered - received %1 fragments, %2 video packets")
          .arg(remotePacketCount)
          .arg(remoteVideoPacketCount));
  qDebug() << "Verified forwarded packet from client" << senderClientId
           << "sequence" << sequenceNumber << "payload" << payloadSize;
}

void MainWindow::onRemoteVideoPacketReceived(std::uint32_t senderClientId,
                                             const QByteArray &encodedPacket) {
  ++remoteVideoPacketCount;
  updateConnectionStatus(
      QString("UDP registered - received %1 fragments, %2 video packets")
          .arg(remotePacketCount)
          .arg(remoteVideoPacketCount));

  auto *decoder = ensureRemoteDecoder(senderClientId);
  if (decoder) {
    decoder->decodePacket(encodedPacket);
  }
}

void MainWindow::sendControlMessage(std::uint8_t type,
                                    const QByteArray &payload) {
  if (!controlSocket ||
      controlSocket->state() != QAbstractSocket::ConnectedState) {
    return;
  }

  controlSocket->write(Protocol::encodeTcpMessage(type, payload));
  controlSocket->flush();
}

void MainWindow::handleControlMessage(const Protocol::TcpMessage &message) {
  switch (message.type) {
  case ControlMessage::Pong:
    qDebug() << "Received pong from server";
    break;
  case ControlMessage::JoinAccepted:
    handleJoinAccepted(message.payload);
    break;
  case ControlMessage::UdpRegistered:
    qDebug() << "UDP media endpoint registered";
    if (udpMediaClient) {
      udpMediaClient->setMediaEnabled(true);
    }
    joinCallButton->setText("Connected");
    updateConnectionStatus("UDP registered - received 0 packets");
    break;
  default:
    qDebug() << "Unknown control message type:" << message.type;
    break;
  }
}

void MainWindow::handleJoinAccepted(const QByteArray &payload) {
  Protocol::JoinAcceptedPayload accepted;
  if (!Protocol::decodeJoinAcceptedPayload(payload, accepted)) {
    qDebug() << "Invalid JoinAccepted payload";
    controlSocket->disconnectFromHost();
    return;
  }

  clientId = accepted.clientId;
  qDebug() << "Joined call as client" << clientId << "UDP port"
           << accepted.udpPort;

  if (!udpMediaClient) {
    udpMediaClient = new UdpMediaClient(this);
    connect(udpMediaClient, &UdpMediaClient::remotePacketReceived, this,
            &MainWindow::onRemotePacketReceived);
    connect(udpMediaClient, &UdpMediaClient::remoteVideoPacketReceived, this,
            &MainWindow::onRemoteVideoPacketReceived);
    connect(udpMediaClient, &UdpMediaClient::packetDropped, this,
            [](const QString &reason) {
              qDebug() << "UDP media packet dropped:" << reason;
            });
    connect(videoFrameHandler, &FrameHandler::encodedPacketAvailable,
            udpMediaClient, &UdpMediaClient::sendVideoPacket,
            Qt::UniqueConnection);
  }

  udpMediaClient->setMediaEnabled(false);
  udpMediaClient->configure(QHostAddress::LocalHost, accepted.udpPort,
                            clientId);
  udpMediaClient->sendRegisterEndpoint();

  joinCallButton->setText("Registering UDP...");
  updateConnectionStatus(QString("TCP joined as client %1").arg(clientId));
}

void MainWindow::updateConnectionStatus(const QString &status) {
  if (statusLabel) {
    statusLabel->setText(status);
  }
}

VideoDecoder *MainWindow::ensureRemoteDecoder(std::uint32_t senderClientId) {
  if (remoteDecoders.contains(senderClientId)) {
    return remoteDecoders.value(senderClientId);
  }

  auto *container = new QWidget(central);
  auto *containerLayout = new QVBoxLayout(container);
  auto *label = new QLabel(QString("Client %1").arg(senderClientId), container);
  auto *preview = new VideoPreview(container);
  preview->setMinimumSize(240, 180);
  containerLayout->addWidget(label);
  containerLayout->addWidget(preview);
  remoteFeedsLayout->addWidget(container);

  auto *decoder = new VideoDecoder(this);
  if (!decoder->isInitialized()) {
    qDebug() << "Failed to initialize H.264 decoder for remote client"
             << senderClientId;
    delete decoder;
    delete container;
    return nullptr;
  }

  connect(decoder, &VideoDecoder::frameDecoded, preview,
          &VideoPreview::updateNextFrame);
  connect(decoder, &VideoDecoder::decodeError, this,
          [senderClientId](const QString &reason) {
            qDebug() << "Remote video decode error for client" << senderClientId
                     << ":" << reason;
          });

  remoteFeedContainers.insert(senderClientId, container);
  remoteDecoders.insert(senderClientId, decoder);
  return decoder;
}

void MainWindow::clearRemoteParticipants() {
  for (auto *decoder : remoteDecoders) {
    delete decoder;
  }
  remoteDecoders.clear();

  for (auto *container : remoteFeedContainers) {
    delete container;
  }
  remoteFeedContainers.clear();
}
