#pragma once

#include "VideoEncoder.h"
#include <QObject>
#include <QVideoFrame>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

struct FrameMetaData;

// ffmpeg AVFrames have at most 8 planes (AV_NUM_DATA_POINTERS)
const int MAX_PLANE_COUNT = 8;
// This is the format that frames received from the camera are converted to
// before they are encoded.
const auto DST_FRAME_FMT = AV_PIX_FMT_YUV420P;

class FrameHandler : public QObject {
  Q_OBJECT

public:
  FrameHandler() {

  };
  ~FrameHandler();
  FrameHandler(const FrameHandler &) = delete;
  FrameHandler &operator=(const FrameHandler &) = delete;
  void enableSingleFrameDevMode();

public slots:
  void receiveFrame(const QVideoFrame &frame);

signals:
  void newFrameAvailable(const QVideoFrame &frame);

private:
  SwsContext *sws_ctx = nullptr;
  bool initializedFrameMetaData = false;
  FrameMetaData frameMetaData;
  // TODO: Find a better approach for setting dst_frame-pts
  // The incrementing of this variable is not atomic and the
  // behavior of this is not guaranteed to match clock time
  // which might affect presentation post encoding on a
  // partner device's end.
  int pts_counter = 0;
  std::unique_ptr<VideoEncoder> videoEncoder;

  /* This is enabled and set to 1 in dev environments when
   * we only want to handle a single frame and observe the
   * results of some operations. It is incremented to 2 after
   * the first frame is received.
   */
  int singleFrameDevModeFlagStatus = 0;

  /* Converts the pixel data into a format from which the video
   * data will be encoded.
   */
  AVFrame *convertPixelFormat(const QVideoFrame &frame);
};
