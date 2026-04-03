#include "FrameHandler.h"

#include <QVideoFrame>

void FrameHandler::enableSingleFrameDevMode() {
  if(singleFrameDevModeFlagStatus < 1) singleFrameDevModeFlagStatus = 1;
}

void FrameHandler::receiveFrame(const QVideoFrame &frame) {
  emit newFrameAvailable(frame); 
  if (singleFrameDevModeFlagStatus && singleFrameDevModeFlagStatus++ > 1)
    return;

  return;
}
