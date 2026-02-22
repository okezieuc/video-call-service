#include "FrameHandler.h"

#include <QVideoFrame>

void FrameHandler::receiveFrame(const QVideoFrame &frame) {
  qInfo("a frame was received");

  return;
}
