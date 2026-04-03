#include "FrameHandler.h"

#include <QImage>
#include <QVideoFrame>
#include <gtest/gtest.h>

namespace {

QVideoFrame makeTestFrame()
{
  QImage image(2, 2, QImage::Format_ARGB32);
  image.fill(qRgba(10, 20, 30, 255));
  image.setPixelColor(1, 0, QColor(40, 50, 60, 255));
  image.setPixelColor(0, 1, QColor(70, 80, 90, 255));
  image.setPixelColor(1, 1, QColor(100, 110, 120, 255));
  return QVideoFrame(image);
}

} // namespace

TEST(FrameHandlerTest, ReceiveFrame_EmitsNewFrameAvailableForValidFrame)
{
  FrameHandler handler;
  int emissionCount = 0;
  QVideoFrame emittedFrame;

  QObject::connect(&handler, &FrameHandler::newFrameAvailable,
                   [&emissionCount, &emittedFrame](const QVideoFrame &frame) {
                     ++emissionCount;
                     emittedFrame = frame;
                   });

  const QVideoFrame inputFrame = makeTestFrame();
  ASSERT_TRUE(inputFrame.isValid());

  handler.receiveFrame(inputFrame);

  EXPECT_EQ(emissionCount, 1);
  EXPECT_TRUE(emittedFrame.isValid());
  EXPECT_EQ(emittedFrame.size(), inputFrame.size());
}

TEST(FrameHandlerTest, ReceiveFrame_DoesNothingForInvalidFrame)
{
  FrameHandler handler;
  int emissionCount = 0;

  QObject::connect(&handler, &FrameHandler::newFrameAvailable,
                   [&emissionCount](const QVideoFrame &) { ++emissionCount; });

  handler.receiveFrame(QVideoFrame());

  EXPECT_EQ(emissionCount, 0);
}

