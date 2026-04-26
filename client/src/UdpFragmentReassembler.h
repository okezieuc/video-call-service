#pragma once

#include "Protocol.h"

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QVector>

#include <cstdint>

class UdpFragmentReassembler {
public:
  enum class Status { Incomplete, Complete, Dropped };

  struct Result {
    Status status = Status::Incomplete;
    QByteArray payload;
    QString dropReason;
  };

  Result accept(const Protocol::UdpPacket &packet);
  void clear();

private:
  struct PendingPacket {
    std::uint16_t fragmentCount = 0;
    QVector<QByteArray> fragments;
    QVector<bool> receivedFragments;
    int receivedCount = 0;
    qsizetype totalSize = 0;
    std::uint64_t lastUpdatedMs = 0;
  };

  static constexpr qsizetype MaxReassembledPayloadSize = 2 * 1024 * 1024;
  static constexpr int MaxPendingPackets = 128;
  static constexpr std::uint64_t PendingPacketTimeoutMs = 5000;

  static quint64 keyFor(std::uint32_t clientId, std::uint32_t frameId);
  void pruneExpired(std::uint64_t nowMs);
  void pruneOverflow();

  QHash<quint64, PendingPacket> m_pendingPackets;
};
