#pragma once

#include "utils.h"
#include "FrameHandler.h"
#include "Protocol.h"
#include "UdpMediaClient.h"
#include "VideoDecoder.h"
#include "VideoPreview.h"

#include <QHash>
#include <QMainWindow>
#include <QMediaCaptureSession>
#include <QPermissions>

// Forward declaration of mock class for tests
class MockMainWindow;

class QCamera;
class QCameraPermission;
class QLabel;
class QPushButton;
class QTcpSocket;
class QVBoxLayout;
class QWidget;
class VideoSink;

struct CameraStatus {
  Tristate isPermissionGranted = Tristate::Unknown;
  Tristate deviceHasCamera = Tristate::Unknown;
  bool isCameraActive = false;
};

class MainWindow : public QMainWindow {
  Q_OBJECT

  friend class MockMainWindow;

public:
  explicit MainWindow(QWidget *parent = nullptr);
  void updateCameraStatus();

protected:
  virtual Qt::PermissionStatus checkCameraPermissionStatus() const;
  virtual void requestCameraPermission();
  virtual void startCamera();
  virtual int videoInputCount() const;

private slots:
  void onJoinCallClicked();
  void onConnected();
  void onDisconnected();
  void onReadyRead();
  void onRemotePacketReceived(std::uint32_t senderClientId,
                              std::uint32_t sequenceNumber,
                              qsizetype payloadSize);
  void onRemoteVideoPacketReceived(std::uint32_t senderClientId,
                                   const QByteArray &encodedPacket);

private:
  void sendControlMessage(std::uint8_t type,
                          const QByteArray &payload = QByteArray());
  void handleControlMessage(const Protocol::TcpMessage &message);
  void handleJoinAccepted(const QByteArray &payload);
  void updateConnectionStatus(const QString &status);
  VideoDecoder *ensureRemoteDecoder(std::uint32_t senderClientId);
  void clearRemoteParticipants();

  // Layout
  QWidget *central = nullptr;
  QVBoxLayout *layout = nullptr;
  QPushButton *joinCallButton = nullptr;
  QLabel *statusLabel = nullptr;
  QLabel *remoteFeedsLabel = nullptr;
  QVBoxLayout *remoteFeedsLayout = nullptr;

  // Camera-related
  QCamera *camera = nullptr;
  QMediaCaptureSession captureSession;
  QCameraPermission cameraPermission;
  QVideoSink *videoSink;

  CameraStatus cameraStatus;

  FrameHandler *videoFrameHandler;
  VideoPreview *videoPreviewArea;

  // Control channel
  QTcpSocket *controlSocket = nullptr;
  QByteArray controlBuffer;
  UdpMediaClient *udpMediaClient = nullptr;
  std::uint32_t clientId = 0;
  std::uint64_t remotePacketCount = 0;
  std::uint64_t remoteVideoPacketCount = 0;
  QHash<std::uint32_t, QWidget *> remoteFeedContainers;
  QHash<std::uint32_t, VideoDecoder *> remoteDecoders;
};
