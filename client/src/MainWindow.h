#pragma once

#include "FrameHandler.h"

#include <QMainWindow>
#include <QMediaCaptureSession>
#include <QPermissions>

class QCamera;
class QCameraPermission;
class QPushButton;
class QVBoxLayout;
class QWidget;
class VideoSink;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);
private:
    // Layout
    QWidget* central = nullptr;
    QVBoxLayout* layout = nullptr;
    QPushButton* joinCallButton = nullptr;

    // Camera-related
    QCamera* camera = nullptr;
    QMediaCaptureSession captureSession;
    QCameraPermission cameraPermission; 
    QVideoSink* videoSink;

    FrameHandler* videoFrameHandler;
};

