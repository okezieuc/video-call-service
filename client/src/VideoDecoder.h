#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QVideoFrame>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

class VideoDecoder : public QObject {
  Q_OBJECT

public:
  explicit VideoDecoder(QObject *parent = nullptr);
  ~VideoDecoder();
  VideoDecoder(const VideoDecoder &) = delete;
  VideoDecoder &operator=(const VideoDecoder &) = delete;

  bool isInitialized() const;

public slots:
  void decodePacket(const QByteArray &encodedPacket);

signals:
  void frameDecoded(const QVideoFrame &frame);
  void decodeError(const QString &reason);

private:
  QVideoFrame convertDecodedFrame(const AVFrame *decodedFrame);

  const AVCodec *codec = nullptr;
  AVCodecContext *ctx = nullptr;
  AVPacket *packet = nullptr;
  AVFrame *frame = nullptr;
  SwsContext *swsCtx = nullptr;
  bool initialized = false;
};
