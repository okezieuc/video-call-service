#pragma once

#include <QObject>
#include <QVideoFrame>

extern "C" {
#include <libavcodec/avcodec.h>
}

class FrameHandler : public QObject {
  Q_OBJECT

public:
  FrameHandler() {

  };
  void enableSingleFrameDevMode();

public slots:
  void receiveFrame(const QVideoFrame &frame);

signals:
  void newFrameAvailable(const QVideoFrame &frame);

private:
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
