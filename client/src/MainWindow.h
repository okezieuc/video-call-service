#pragma once

#include <QMainWindow>
#include <QMediaCaptureSession>
#include <QPermissions>

class QCamera;
class QCameraPermission;
class QPushButton;
class QVBoxLayout;
class QVideoWidget;
class QWidget;

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
    QVideoWidget* videoPreview = nullptr;
    QCameraPermission cameraPermission; 
};

