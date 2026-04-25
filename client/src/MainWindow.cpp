#include "MainWindow.h"
#include "FrameHandler.h"
#include "SharedTypes.h"
#include "VideoPreview.h"

#include <QApplication>
#include <QCamera>
#include <QCameraDevice>
#include <QDebug>
#include <QImageCapture>
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
  videoPreviewArea = new VideoPreview();
  layout->addWidget(joinCallButton);
  layout->addWidget(videoPreviewArea);
  setCentralWidget(central);
  setWindowTitle("Video Call Client");
  resize(480, 320);

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
  controlSocket->connectToHost("localhost", 5555);
}

void MainWindow::onConnected() {
  qDebug() << "Connected to server; sending ping";

  QByteArray message;
  message.append(static_cast<char>(ControlMessage::Ping));
  controlSocket->write(message);
  controlSocket->flush();

  joinCallButton->setText("Waiting for Pong...");
}

void MainWindow::onDisconnected() {
  qDebug() << "Disconnected from server";
  joinCallButton->setText("Join Call");
}

void MainWindow::onReadyRead() {
  const QByteArray data = controlSocket->readAll();

  for (const char byte : data) {
    const auto type = static_cast<std::uint8_t>(byte);
    if (type == ControlMessage::Pong) {
      qDebug() << "Received pong from server";
      joinCallButton->setText("Connected");
    } else {
      qDebug() << "Unknown control message type:" << type;
    }
  }
}
