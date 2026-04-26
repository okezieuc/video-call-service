#include "VideoDecoder.h"

extern "C" {
#include <libavutil/imgutils.h>
}

#include <QImage>

#include <cstring>

VideoDecoder::VideoDecoder(QObject *parent) : QObject(parent) {
  codec = avcodec_find_decoder(AV_CODEC_ID_H264);
  if (!codec) {
    return;
  }

  ctx = avcodec_alloc_context3(codec);
  if (!ctx) {
    return;
  }

  if (avcodec_open2(ctx, codec, nullptr) < 0) {
    avcodec_free_context(&ctx);
    return;
  }

  packet = av_packet_alloc();
  frame = av_frame_alloc();
  if (!packet || !frame) {
    return;
  }

  initialized = true;
}

VideoDecoder::~VideoDecoder() {
  if (swsCtx) {
    sws_freeContext(swsCtx);
  }
  if (frame) {
    av_frame_free(&frame);
  }
  if (packet) {
    av_packet_free(&packet);
  }
  if (ctx) {
    avcodec_free_context(&ctx);
  }
}

bool VideoDecoder::isInitialized() const { return initialized; }

void VideoDecoder::decodePacket(const QByteArray &encodedPacket) {
  if (!initialized || encodedPacket.isEmpty()) {
    return;
  }

  av_packet_unref(packet);
  if (av_new_packet(packet, encodedPacket.size()) < 0) {
    emit decodeError("failed to allocate decoder packet");
    return;
  }
  std::memcpy(packet->data, encodedPacket.constData(), encodedPacket.size());

  int ret = avcodec_send_packet(ctx, packet);
  av_packet_unref(packet);
  if (ret < 0) {
    emit decodeError("failed to send H.264 packet to decoder");
    return;
  }

  while (true) {
    ret = avcodec_receive_frame(ctx, frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      break;
    }

    if (ret < 0) {
      emit decodeError("failed to decode H.264 frame");
      break;
    }

    const auto videoFrame = convertDecodedFrame(frame);
    if (videoFrame.isValid()) {
      emit frameDecoded(videoFrame);
    }
    av_frame_unref(frame);
  }
}

QVideoFrame VideoDecoder::convertDecodedFrame(const AVFrame *decodedFrame) {
  if (!decodedFrame || decodedFrame->width <= 0 || decodedFrame->height <= 0) {
    return QVideoFrame();
  }

  QImage image(decodedFrame->width, decodedFrame->height,
               QImage::Format_ARGB32);
  if (image.isNull()) {
    return QVideoFrame();
  }

  swsCtx = sws_getCachedContext(
      swsCtx, decodedFrame->width, decodedFrame->height,
      static_cast<AVPixelFormat>(decodedFrame->format), decodedFrame->width,
      decodedFrame->height, AV_PIX_FMT_BGRA, SWS_BILINEAR, nullptr, nullptr,
      nullptr);
  if (!swsCtx) {
    emit decodeError("failed to create decoder color conversion context");
    return QVideoFrame();
  }

  uint8_t *destinationData[4] = {image.bits(), nullptr, nullptr, nullptr};
  int destinationLineSize[4] = {static_cast<int>(image.bytesPerLine()), 0, 0,
                                0};
  sws_scale(swsCtx, decodedFrame->data, decodedFrame->linesize, 0,
            decodedFrame->height, destinationData, destinationLineSize);

  return QVideoFrame(image);
}
