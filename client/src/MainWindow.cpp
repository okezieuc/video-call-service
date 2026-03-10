#include "MainWindow.h"
#include "FrameHandler.h"
#include "VideoPreview.h"

#include <QApplication>
#include <QCamera>
#include <QCameraDevice>
#include <QImageCapture>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QPainter>
#include <QPermissions>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVideoSink>
#include <QWidget>

bool MainWindow::isCameraAvailable() const {
  if (QMediaDevices::videoInputs().count() > 0)
    return true;
  else
    return false;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  if (!isCameraAvailable()) {
    qInfo("A camera is not available");
    return;
  }

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
  connect(videoSink, &QVideoSink::videoFrameChanged, videoFrameHandler,
          &FrameHandler::receiveFrame);
  connect(videoFrameHandler, &FrameHandler::newFrameAvailable, videoPreviewArea,
          &VideoPreview::updateNextFrame);

  camera = new QCamera;
  captureSession.setCamera(camera);
  captureSession.setVideoOutput(videoSink);

  switch (qApp->checkPermission(cameraPermission)) {
  case Qt::PermissionStatus::Undetermined:
    qApp->requestPermission(cameraPermission, camera, &QCamera::start);
    return;
  case Qt::PermissionStatus::Denied:
    qInfo("This app does not have the permission to access the camera.");
    return;
  case Qt::PermissionStatus::Granted:
    camera->start();
    return;
  }

  connect(joinCallButton, &QPushButton::clicked, this, []() {});
}
