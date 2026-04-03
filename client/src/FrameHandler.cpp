#include "FrameHandler.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

#include <QVideoFrame>

void FrameHandler::enableSingleFrameDevMode() {
  if (singleFrameDevModeFlagStatus < 1)
    singleFrameDevModeFlagStatus = 1;
}

void FrameHandler::receiveFrame(const QVideoFrame &receivedFrame) {
  if (singleFrameDevModeFlagStatus && singleFrameDevModeFlagStatus++ > 1)
    return;

  QVideoFrame frame(receivedFrame);

  if (!frame.map(QVideoFrame::MapMode::ReadOnly))
    return;

  auto converted_frame = convertPixelFormat(frame);
  frame.unmap();

  // TODO: What else do we need to do when handling this
  if (converted_frame == nullptr)
    return;

  av_frame_free(&converted_frame);
  emit newFrameAvailable(frame);

  return;
}

AVFrame *FrameHandler::convertPixelFormat(const QVideoFrame &frame) {
  int frameWidth = frame.width(), frameHeight = frame.height();

  // TODO: Replace hardcoded source format with a dynamic check
  auto sourceFormat = AV_PIX_FMT_BGRA;
  auto destinationFormat = AV_PIX_FMT_YUV420P;

  // TODO: Create a shared context once and reuse it across conversions
  SwsContext *sws_ctx = sws_getContext(
      frameWidth, frameHeight, sourceFormat, frameWidth, frameHeight,
      destinationFormat, SWS_BILINEAR, NULL, NULL, NULL);

  if (!sws_ctx) {
    // TODO: Figure out a more sane way for handling this error
    qDebug("Could not create scaling context");
    sws_free_context(&sws_ctx);
    return nullptr;
  }

  AVFrame *src_frame = av_frame_alloc();
  if (!src_frame) {
    return nullptr;
  }

  AVFrame *dst_frame = av_frame_alloc();
  if (!dst_frame) {
    av_frame_free(&src_frame);
    return nullptr;
  }

  // TODO: Update this to handle multiple formats with multiple planes
  src_frame->format = sourceFormat;
  src_frame->width = frameWidth;
  src_frame->height = frameHeight;
  src_frame->data[0] = const_cast<uint8_t *>(frame.bits(0));
  src_frame->linesize[0] = frame.bytesPerLine(0);

  dst_frame->format = destinationFormat;
  dst_frame->width = frameWidth;
  dst_frame->height = frameHeight;
  av_frame_get_buffer(dst_frame, 0);

  int res = sws_scale_frame(sws_ctx, dst_frame, src_frame);
  if (res < 0) {
    qDebug("Conversion failed");
    av_frame_free(&dst_frame);
    av_frame_free(&src_frame);
    sws_free_context(&sws_ctx);
    return nullptr;
  }

  // TODO: Remember to call av_frame_free on dst_frame
  av_frame_free(&src_frame);
  sws_free_context(&sws_ctx);

  return dst_frame;
}
