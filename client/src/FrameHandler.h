#pragma once

#include <QObject>
#include <QVideoFrame>

class FrameHandler : public QObject {
  Q_OBJECT

public:
  FrameHandler() = default;
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
};
