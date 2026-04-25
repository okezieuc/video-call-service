#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <QByteArray>
#include <QVector>

struct FrameMetaData {
  int width;
  int height;
  AVPixelFormat src_fmt;
  int plane_count;
  int linesize[8];
};

class VideoEncoder {
public:
  VideoEncoder(FrameMetaData metadata);
  ~VideoEncoder();
  VideoEncoder(const VideoEncoder &) = delete;
  VideoEncoder &operator=(const VideoEncoder &) = delete;
  QVector<QByteArray> encodeFrame(AVFrame *frame);
  bool isInitialized();

private:
  const AVCodec *codec = nullptr;
  AVCodecContext *ctx = nullptr;
  AVPacket *pkt = nullptr;

  // Some things in the constructor can error. If a part of the construction of
  // this object fails and the object is left indeterminate state, the value of
  // this will be left as false. Otherwise, it will be set to true upon the
  // completion of the construction of the object.
  bool initialized = false;
};
