#pragma once

#include <QObject>
#include <QVideoFrame>
#include <QWidget>

class QPaintEvent;

class VideoPreview : public QWidget {
  Q_OBJECT

public:
  VideoPreview(QWidget *parent = nullptr) : QWidget(parent) {}

public slots:
  void updateNextFrame(const QVideoFrame &frame);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  QVideoFrame currentFrame;
};
