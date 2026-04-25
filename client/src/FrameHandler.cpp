#include "FrameHandler.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

#include <QVideoFrame>

FrameHandler::~FrameHandler() {
  if (sws_ctx) {
    sws_free_context(&sws_ctx);
  }
}

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

  // TODO: The frame metadata is not guaranteed to remain fixed over the
  // lifetime of the program. Instead of cacing the metadata permanently, only
  // cache the sws_ctx which is expensive to recreate and only recreate it
  // whenever the frame metadata changes.
  if (!initializedFrameMetaData) {
    frameMetaData = FrameMetaData{frame.width(), frame.height(),
                                  AV_PIX_FMT_BGRA, frame.planeCount()};

    for (int i = 0; i < frame.planeCount(); i++) {
      frameMetaData.linesize[i] = frame.bytesPerLine(i);
    }

    initializedFrameMetaData = true;
  }

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
  int width = frameMetaData.width, height = frameMetaData.height;

  // TODO: Replace hardcoded source format with a dynamic check
  auto sourceFormat = AV_PIX_FMT_BGRA;
  auto destinationFormat = DST_FRAME_FMT;

  // Create a scaling context if one was not previously created
  if (!sws_ctx) {
    sws_ctx = sws_getContext(width, height, sourceFormat, width, height,
                             destinationFormat, SWS_BILINEAR, NULL, NULL, NULL);

    if (!sws_ctx) {
      // TODO: Figure out a more sane way for handling this error
      qDebug("Could not create scaling context");
      sws_free_context(&sws_ctx);
      return nullptr;
    }
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
  src_frame->width = width;
  src_frame->height = height;
  for (int i = 0; i < frameMetaData.plane_count; i++) {
    src_frame->data[i] = const_cast<uint8_t *>(frame.bits(i));
    src_frame->linesize[i] = frame.bytesPerLine(i);
  }

  dst_frame->format = destinationFormat;
  dst_frame->width = width;
  dst_frame->height = height;
  dst_frame->pts = ++pts_counter;
  if (av_frame_get_buffer(dst_frame, 0) < 0) {
    av_frame_free(&dst_frame);
    av_frame_free(&src_frame);
    return nullptr;
  }

  int res = sws_scale_frame(sws_ctx, dst_frame, src_frame);
  if (res < 0) {
    qDebug("Conversion failed");
    av_frame_free(&dst_frame);
    av_frame_free(&src_frame);
    return nullptr;
  }

  if (!videoEncoder) {
    videoEncoder = std::make_unique<VideoEncoder>(frameMetaData);
  }

  // TODO: Is this the best way to handle the case where the video encoder
  // is not properly initialized?
  if (videoEncoder->isInitialized()) {
    const auto packets = videoEncoder->encodeFrame(dst_frame);
    for (const auto &packet : packets) {
      emit encodedPacketAvailable(packet);
    }
  }

  // TODO: Remember to call av_frame_free on dst_frame
  av_frame_free(&src_frame);

  return dst_frame;
}
