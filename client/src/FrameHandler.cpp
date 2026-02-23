#include "FrameHandler.h"

#include <QVideoFrame>

void FrameHandler::receiveFrame(const QVideoFrame &frame) {
  emit newFrameAvailable(frame); 
  return;
}
