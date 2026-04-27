#include "FrameHandler.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

#include <QImage>
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

  if (!receivedFrame.isValid()) {
    return;
  }

  QImage image = receivedFrame.toImage();
  if (image.isNull()) {
    return;
  }

  image = image.convertToFormat(QImage::Format_RGBA8888);

  const int evenWidth = image.width() & ~1;
  const int evenHeight = image.height() & ~1;
  if (evenWidth <= 0 || evenHeight <= 0) {
    return;
  }

  if (evenWidth != image.width() || evenHeight != image.height()) {
    image = image.copy(0, 0, evenWidth, evenHeight);
  }

  auto converted_frame = convertPixelFormat(image);

  if (converted_frame == nullptr)
    return;

  av_frame_free(&converted_frame);
  emit newFrameAvailable(QVideoFrame(image));

  return;
}

AVFrame *FrameHandler::convertPixelFormat(const QImage &image) {
  const int width = image.width();
  const int height = image.height();
  auto sourceFormat = AV_PIX_FMT_RGBA;
  auto destinationFormat = DST_FRAME_FMT;

  const bool metadataChanged =
      !initializedFrameMetaData || frameMetaData.width != width ||
      frameMetaData.height != height || frameMetaData.src_fmt != sourceFormat;
  if (metadataChanged) {
    frameMetaData = FrameMetaData{width, height, sourceFormat, 1, {0}};
    frameMetaData.linesize[0] = image.bytesPerLine();
    initializedFrameMetaData = true;
    videoEncoder.reset();
  }

  sws_ctx = sws_getCachedContext(sws_ctx, width, height, sourceFormat, width,
                                 height, destinationFormat, SWS_BILINEAR, NULL,
                                 NULL, NULL);
  if (!sws_ctx) {
    qDebug("Could not create scaling context");
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

  src_frame->format = sourceFormat;
  src_frame->width = width;
  src_frame->height = height;
  src_frame->data[0] = const_cast<uint8_t *>(image.constBits());
  src_frame->linesize[0] = image.bytesPerLine();

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

  av_frame_free(&src_frame);

  return dst_frame;
}
