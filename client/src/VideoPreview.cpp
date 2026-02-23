#include "VideoPreview.h"

#include <QPainter>
#include <QVideoFrame>

void VideoPreview::updateNextFrame(const QVideoFrame &frame) {
  if(!frame.isValid()) return;

  currentFrame = frame;
  update();
  return;
}

void VideoPreview::paintEvent(QPaintEvent *event) {
  if(!currentFrame.isValid()) return;

  QPainter painter(this);

  QVideoFrame::PaintOptions opt{};

  currentFrame.paint(&painter, this->rect(), opt);
}
