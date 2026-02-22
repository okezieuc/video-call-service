#include "MainWindow.h"

#include "iostream"
#include <QApplication>
#include <QMediaDevices>
#include <QImageCapture>
#include <QMediaCaptureSession>
#include <QCamera>
#include <QCameraDevice>
#include <QPermissions>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVideoWidget>
#include <QWidget>

bool checkCameraAvailability() {
  if(QMediaDevices::videoInputs().count() > 0)
    return true;
  else
    return false;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    central = new QWidget(this);
    layout = new QVBoxLayout(central);
    joinCallButton = new QPushButton("Join Call", central);
    videoPreview = new QVideoWidget();

    layout->addWidget(videoPreview);
    layout->addWidget(joinCallButton);
    setCentralWidget(central);
    setWindowTitle("Video Call Client");
    resize(480, 320);

    if(!checkCameraAvailability()) {
      qInfo("A camera is not available");
    }
    else { 
        camera = new QCamera;
        captureSession.setCamera(camera);
        captureSession.setVideoOutput(videoPreview);
        videoPreview -> show();

        switch(qApp->checkPermission(cameraPermission)) {
          case Qt::PermissionStatus::Undetermined:
            qApp->requestPermission(cameraPermission, camera, &QCamera::start);
              return;
          case Qt::PermissionStatus::Denied:
            return;
          case Qt::PermissionStatus::Granted:
            camera -> start();
            return;
        }
    }

    connect(joinCallButton, &QPushButton::clicked, this, []() {});
}
