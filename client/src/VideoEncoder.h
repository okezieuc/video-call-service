#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
}

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
  int encodeFrame(AVFrame *frame);
  bool isInitialized();

private:
  const AVCodec *codec;
  AVCodecContext *ctx;
  AVPacket *pkt;

  // Some things in the constructor can error. If a part of the construction of
  // this object fails and the object is left indeterminate state, the value of
  // this will be left as false. Otherwise, it will be set to true upon the
  // completion of the construction of the object.
  bool initialized = false;
};
