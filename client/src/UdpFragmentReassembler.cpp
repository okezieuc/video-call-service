#include "UdpFragmentReassembler.h"

#include <limits>

UdpFragmentReassembler::Result
UdpFragmentReassembler::accept(const Protocol::UdpPacket &packet) {
  Result result;

  if (packet.header.packetType != Protocol::UdpPacketType::VideoPacket) {
    result.status = Status::Dropped;
    result.dropReason = "unexpected UDP packet type for video reassembly";
    return result;
  }

  const auto fragmentCount = packet.header.fragmentCount;
  const auto fragmentIndex = packet.header.fragmentIndex;
  if (fragmentCount == 0 || fragmentIndex >= fragmentCount) {
    result.status = Status::Dropped;
    result.dropReason = "invalid UDP video fragment metadata";
    return result;
  }

  if (static_cast<qsizetype>(fragmentCount) *
          Protocol::MaxUdpPayloadSize >
      MaxReassembledPayloadSize) {
    result.status = Status::Dropped;
    result.dropReason = "UDP video packet exceeds reassembly limit";
    return result;
  }

  if (fragmentCount == 1) {
    result.status = Status::Complete;
    result.payload = packet.payload;
    return result;
  }

  const auto nowMs = Protocol::currentTimestampMs();
  pruneExpired(nowMs);

  const auto key = keyFor(packet.header.clientId, packet.header.frameId);
  auto packetIt = m_pendingPackets.find(key);
  if (packetIt == m_pendingPackets.end()) {
    PendingPacket pending;
    pending.fragmentCount = fragmentCount;
    pending.fragments.resize(fragmentCount);
    pending.receivedFragments.resize(fragmentCount);
    pending.lastUpdatedMs = nowMs;
    packetIt = m_pendingPackets.insert(key, pending);
  }

  PendingPacket &pending = packetIt.value();
  if (pending.fragmentCount != fragmentCount) {
    m_pendingPackets.erase(packetIt);
    result.status = Status::Dropped;
    result.dropReason = "conflicting UDP video fragment count";
    return result;
  }

  if (pending.receivedFragments[fragmentIndex]) {
    pending.lastUpdatedMs = nowMs;
    return result;
  }

  if (pending.totalSize + packet.payload.size() > MaxReassembledPayloadSize) {
    m_pendingPackets.erase(packetIt);
    result.status = Status::Dropped;
    result.dropReason = "UDP video packet exceeds reassembly limit";
    return result;
  }

  pending.fragments[fragmentIndex] = packet.payload;
  pending.receivedFragments[fragmentIndex] = true;
  ++pending.receivedCount;
  pending.totalSize += packet.payload.size();
  pending.lastUpdatedMs = nowMs;

  if (pending.receivedCount != pending.fragmentCount) {
    pruneOverflow();
    return result;
  }

  QByteArray payload;
  payload.reserve(pending.totalSize);
  for (const auto &fragment : pending.fragments) {
    payload.append(fragment);
  }
  m_pendingPackets.erase(packetIt);

  result.status = Status::Complete;
  result.payload = payload;
  return result;
}

void UdpFragmentReassembler::clear() { m_pendingPackets.clear(); }

quint64 UdpFragmentReassembler::keyFor(std::uint32_t clientId,
                                       std::uint32_t frameId) {
  return (static_cast<quint64>(clientId) << 32) |
         static_cast<quint64>(frameId);
}

void UdpFragmentReassembler::pruneExpired(std::uint64_t nowMs) {
  for (auto it = m_pendingPackets.begin(); it != m_pendingPackets.end();) {
    if (nowMs - it.value().lastUpdatedMs > PendingPacketTimeoutMs) {
      it = m_pendingPackets.erase(it);
    } else {
      ++it;
    }
  }
}

void UdpFragmentReassembler::pruneOverflow() {
  while (m_pendingPackets.size() > MaxPendingPackets) {
    auto oldestIt = m_pendingPackets.begin();
    for (auto it = m_pendingPackets.begin(); it != m_pendingPackets.end();
         ++it) {
      if (it.value().lastUpdatedMs < oldestIt.value().lastUpdatedMs) {
        oldestIt = it;
      }
    }
    m_pendingPackets.erase(oldestIt);
  }
}
