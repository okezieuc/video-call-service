#pragma once

#include "utils.h"
#include "FrameHandler.h"
#include "VideoPreview.h"

#include <QMainWindow>
#include <QMediaCaptureSession>
#include <QPermissions>

// Forward declaration of mock class for tests
class MockMainWindow;

class QCamera;
class QCameraPermission;
class QPushButton;
class QVBoxLayout;
class QWidget;
class VideoSink;

struct CameraStatus {
  Tristate isPermissionGranted = Tristate::Unknown;
  Tristate deviceHasCamera = Tristate::Unknown;
  bool isCameraActive = false;
};

class MainWindow : public QMainWindow {
  friend class MockMainWindow;

public:
  explicit MainWindow(QWidget *parent = nullptr);
  void updateCameraStatus();

protected:
  virtual Qt::PermissionStatus checkCameraPermissionStatus() const;
  virtual void requestCameraPermission();
  virtual void startCamera();
  virtual int videoInputCount() const;

private:
  // Layout
  QWidget *central = nullptr;
  QVBoxLayout *layout = nullptr;
  QPushButton *joinCallButton = nullptr;

  // Camera-related
  QCamera *camera = nullptr;
  QMediaCaptureSession captureSession;
  QCameraPermission cameraPermission;
  QVideoSink *videoSink;

  CameraStatus cameraStatus;

  FrameHandler *videoFrameHandler;
  VideoPreview *videoPreviewArea;
};
