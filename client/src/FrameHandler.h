#pragma once

#include <QObject>
#include <QVideoFrame>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

// ffmpeg AVFrames have at most 8 planes (AV_NUM_DATA_POINTERS)
const int MAX_PLANE_COUNT = 8;

struct FrameMetaData {
  int width;
  int height;
  AVPixelFormat src_fmt;
  int plane_count;
  int linesize[8];
};

class FrameHandler : public QObject {
  Q_OBJECT

public:
  FrameHandler() {

  };
  ~FrameHandler();
  FrameHandler(const FrameHandler&) = delete;
  FrameHandler& operator=(const FrameHandler&) = delete;
  void enableSingleFrameDevMode();

public slots:
  void receiveFrame(const QVideoFrame &frame);

signals:
  void newFrameAvailable(const QVideoFrame &frame);

private:
  SwsContext *sws_ctx = nullptr;
  bool initializedFrameMetaData = false;
  FrameMetaData frameMetaData;

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
