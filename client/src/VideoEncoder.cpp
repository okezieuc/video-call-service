#include "VideoEncoder.h"

VideoEncoder::VideoEncoder(FrameMetaData metadata) {
  codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  if (!codec) {
    initialized = false;
    return;
  }

  ctx = avcodec_alloc_context3(codec);
  if (!ctx) {
    initialized = false;
    return;
  }

  ctx->width = metadata.width;
  ctx->height = metadata.height;
  // TODO: Create a file with shared constants like the one used below
  ctx->pix_fmt = AV_PIX_FMT_YUV420P;
  ctx->time_base = (AVRational){1, 30};
  ctx->framerate = (AVRational){30, 1};
  // TODO: These default values are arbitrary. Experiment to figure out a more
  // reasonable set of values.
  ctx->bit_rate = 8000000;
  ctx->gop_size = 30;
  ctx->max_b_frames = 0;

  // initialize the codec context to use the provided codec
  if (avcodec_open2(ctx, codec, NULL) < 0) {
    initialized = false;
    avcodec_free_context(&ctx);
    return;
  }

  pkt = av_packet_alloc();
  if (!pkt) {
    initialized = false;
    avcodec_free_context(&ctx);
    return;
  }

  initialized = true;
}

VideoEncoder::~VideoEncoder() {
  if (ctx)
    avcodec_free_context(&ctx);
  if (pkt)
    av_packet_unref(pkt);
}

int VideoEncoder::encodeFrame(AVFrame *frame) {
  if (!initialized)
    return -1;

  int ret = avcodec_send_frame(ctx, frame);
  if (ret < 0) {
    return ret;
  }

  while (true) {
    ret = avcodec_receive_packet(ctx, pkt);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
      break;

    if (ret < 0)
      return ret;
  }

  return 0;
}

bool VideoEncoder::isInitialized() { return initialized; }
