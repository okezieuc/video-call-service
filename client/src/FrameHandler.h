#pragma once

#include <QObject>
#include <QVideoFrame>

class FrameHandler : public QObject {
  Q_OBJECT

public:
  FrameHandler() = default;

public slots:
  void receiveFrame(const QVideoFrame &frame);
};
