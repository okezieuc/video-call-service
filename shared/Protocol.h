#pragma once

#include "SharedTypes.h"

#include <cstdint>

#include <QByteArray>
#include <QDateTime>

namespace Protocol {

constexpr std::uint32_t UdpMagic = 0x56435331; // VCS1
constexpr std::uint8_t Version = 1;
constexpr qsizetype TcpHeaderSize = 5;
constexpr qsizetype UdpHeaderSize = 34;
constexpr qsizetype MaxUdpDatagramSize = 1200;
constexpr qsizetype MaxUdpPayloadSize = MaxUdpDatagramSize - UdpHeaderSize;
constexpr std::uint32_t MaxTcpPayloadSize = 64 * 1024;

namespace UdpPacketType {
constexpr std::uint8_t RegisterEndpoint = 0x01;
constexpr std::uint8_t VideoPacket = 0x02;
constexpr std::uint8_t ReceiverStats = 0x03;
} // namespace UdpPacketType

struct TcpMessage {
  std::uint8_t type = 0;
  QByteArray payload;
};

struct JoinAcceptedPayload {
  std::uint32_t clientId = 0;
  std::uint16_t udpPort = 0;
};

struct UdpHeader {
  std::uint8_t packetType = 0;
  std::uint32_t clientId = 0;
  std::uint32_t sequenceNumber = 0;
  std::uint64_t timestampMs = 0;
  std::uint16_t flags = 0;
  std::uint16_t payloadLength = 0;
  std::uint32_t frameId = 0;
  std::uint16_t fragmentIndex = 0;
  std::uint16_t fragmentCount = 1;
};

struct UdpPacket {
  UdpHeader header;
  QByteArray payload;
};

enum class DecodeResult { Success, Incomplete, Invalid };

inline void appendUint8(QByteArray &data, std::uint8_t value) {
  data.append(static_cast<char>(value));
}

inline void appendUint16(QByteArray &data, std::uint16_t value) {
  data.append(static_cast<char>((value >> 8) & 0xff));
  data.append(static_cast<char>(value & 0xff));
}

inline void appendUint32(QByteArray &data, std::uint32_t value) {
  data.append(static_cast<char>((value >> 24) & 0xff));
  data.append(static_cast<char>((value >> 16) & 0xff));
  data.append(static_cast<char>((value >> 8) & 0xff));
  data.append(static_cast<char>(value & 0xff));
}

inline void appendUint64(QByteArray &data, std::uint64_t value) {
  appendUint32(data, static_cast<std::uint32_t>(value >> 32));
  appendUint32(data, static_cast<std::uint32_t>(value & 0xffffffff));
}

inline std::uint8_t readUint8(const QByteArray &data, qsizetype offset) {
  return static_cast<std::uint8_t>(data.at(offset));
}

inline std::uint16_t readUint16(const QByteArray &data, qsizetype offset) {
  return (static_cast<std::uint16_t>(
              static_cast<unsigned char>(data.at(offset)))
          << 8) |
         static_cast<std::uint16_t>(
             static_cast<unsigned char>(data.at(offset + 1)));
}

inline std::uint32_t readUint32(const QByteArray &data, qsizetype offset) {
  return (static_cast<std::uint32_t>(
              static_cast<unsigned char>(data.at(offset)))
          << 24) |
         (static_cast<std::uint32_t>(
              static_cast<unsigned char>(data.at(offset + 1)))
          << 16) |
         (static_cast<std::uint32_t>(
              static_cast<unsigned char>(data.at(offset + 2)))
          << 8) |
         static_cast<std::uint32_t>(
             static_cast<unsigned char>(data.at(offset + 3)));
}

inline std::uint64_t readUint64(const QByteArray &data, qsizetype offset) {
  return (static_cast<std::uint64_t>(readUint32(data, offset)) << 32) |
         static_cast<std::uint64_t>(readUint32(data, offset + 4));
}

inline QByteArray encodeTcpMessage(std::uint8_t type,
                                   const QByteArray &payload = QByteArray()) {
  QByteArray data;
  appendUint8(data, type);
  appendUint32(data, static_cast<std::uint32_t>(payload.size()));
  data.append(payload);
  return data;
}

inline DecodeResult takeTcpMessage(QByteArray &buffer, TcpMessage &message) {
  if (buffer.size() < TcpHeaderSize) {
    return DecodeResult::Incomplete;
  }

  const auto payloadLength = readUint32(buffer, 1);
  if (payloadLength > MaxTcpPayloadSize) {
    return DecodeResult::Invalid;
  }

  const auto frameLength =
      TcpHeaderSize + static_cast<qsizetype>(payloadLength);
  if (buffer.size() < frameLength) {
    return DecodeResult::Incomplete;
  }

  message.type = readUint8(buffer, 0);
  message.payload = buffer.mid(TcpHeaderSize, payloadLength);
  buffer.remove(0, frameLength);
  return DecodeResult::Success;
}

inline QByteArray encodeJoinAcceptedPayload(const JoinAcceptedPayload &payload) {
  QByteArray data;
  appendUint32(data, payload.clientId);
  appendUint16(data, payload.udpPort);
  return data;
}

inline bool decodeJoinAcceptedPayload(const QByteArray &data,
                                      JoinAcceptedPayload &payload) {
  if (data.size() != 6) {
    return false;
  }

  payload.clientId = readUint32(data, 0);
  payload.udpPort = readUint16(data, 4);
  return true;
}

inline QByteArray encodeUdpPacket(const UdpPacket &packet) {
  QByteArray data;
  appendUint32(data, UdpMagic);
  appendUint8(data, Version);
  appendUint8(data, packet.header.packetType);
  appendUint32(data, packet.header.clientId);
  appendUint32(data, packet.header.sequenceNumber);
  appendUint64(data, packet.header.timestampMs);
  appendUint16(data, packet.header.flags);
  appendUint16(data, static_cast<std::uint16_t>(packet.payload.size()));
  appendUint32(data, packet.header.frameId);
  appendUint16(data, packet.header.fragmentIndex);
  appendUint16(data, packet.header.fragmentCount);
  data.append(packet.payload);
  return data;
}

inline bool decodeUdpPacket(const QByteArray &data, UdpPacket &packet) {
  if (data.size() < UdpHeaderSize) {
    return false;
  }

  if (readUint32(data, 0) != UdpMagic || readUint8(data, 4) != Version) {
    return false;
  }

  packet.header.packetType = readUint8(data, 5);
  packet.header.clientId = readUint32(data, 6);
  packet.header.sequenceNumber = readUint32(data, 10);
  packet.header.timestampMs = readUint64(data, 14);
  packet.header.flags = readUint16(data, 22);
  packet.header.payloadLength = readUint16(data, 24);
  packet.header.frameId = readUint32(data, 26);
  packet.header.fragmentIndex = readUint16(data, 30);
  packet.header.fragmentCount = readUint16(data, 32);

  if (data.size() != UdpHeaderSize + packet.header.payloadLength) {
    return false;
  }

  if (packet.header.fragmentCount == 0 ||
      packet.header.fragmentIndex >= packet.header.fragmentCount) {
    return false;
  }

  packet.payload = data.mid(UdpHeaderSize, packet.header.payloadLength);
  return true;
}

inline std::uint64_t currentTimestampMs() {
  return static_cast<std::uint64_t>(QDateTime::currentMSecsSinceEpoch());
}

} // namespace Protocol
