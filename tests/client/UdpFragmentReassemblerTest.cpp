#include "UdpFragmentReassembler.h"

#include <gtest/gtest.h>

namespace {

Protocol::UdpPacket makeFragment(std::uint32_t clientId, std::uint32_t frameId,
                                 std::uint16_t fragmentIndex,
                                 std::uint16_t fragmentCount,
                                 const QByteArray &payload) {
  Protocol::UdpPacket packet;
  packet.header.packetType = Protocol::UdpPacketType::VideoPacket;
  packet.header.clientId = clientId;
  packet.header.sequenceNumber = fragmentIndex + 1;
  packet.header.timestampMs = 1234;
  packet.header.frameId = frameId;
  packet.header.fragmentIndex = fragmentIndex;
  packet.header.fragmentCount = fragmentCount;
  packet.payload = payload;
  return packet;
}

} // namespace

TEST(UdpFragmentReassemblerTest, Accept_CompletesSingleFragmentPacket) {
  UdpFragmentReassembler reassembler;

  const auto result =
      reassembler.accept(makeFragment(7, 12, 0, 1, QByteArray("encoded")));

  EXPECT_EQ(result.status, UdpFragmentReassembler::Status::Complete);
  EXPECT_EQ(result.payload, QByteArray("encoded"));
}

TEST(UdpFragmentReassemblerTest, Accept_ReassemblesOutOfOrderFragments) {
  UdpFragmentReassembler reassembler;

  auto result = reassembler.accept(makeFragment(7, 12, 1, 3, "bb"));
  EXPECT_EQ(result.status, UdpFragmentReassembler::Status::Incomplete);

  result = reassembler.accept(makeFragment(7, 12, 2, 3, "cc"));
  EXPECT_EQ(result.status, UdpFragmentReassembler::Status::Incomplete);

  result = reassembler.accept(makeFragment(7, 12, 0, 3, "aa"));
  EXPECT_EQ(result.status, UdpFragmentReassembler::Status::Complete);
  EXPECT_EQ(result.payload, QByteArray("aabbcc"));
}

TEST(UdpFragmentReassemblerTest, Accept_IgnoresDuplicateFragments) {
  UdpFragmentReassembler reassembler;

  auto result = reassembler.accept(makeFragment(7, 12, 0, 2, "aa"));
  EXPECT_EQ(result.status, UdpFragmentReassembler::Status::Incomplete);

  result = reassembler.accept(makeFragment(7, 12, 0, 2, "aa"));
  EXPECT_EQ(result.status, UdpFragmentReassembler::Status::Incomplete);

  result = reassembler.accept(makeFragment(7, 12, 1, 2, "bb"));
  EXPECT_EQ(result.status, UdpFragmentReassembler::Status::Complete);
  EXPECT_EQ(result.payload, QByteArray("aabb"));
}

TEST(UdpFragmentReassemblerTest, Accept_DropsConflictingFragmentCount) {
  UdpFragmentReassembler reassembler;

  auto result = reassembler.accept(makeFragment(7, 12, 0, 2, "aa"));
  EXPECT_EQ(result.status, UdpFragmentReassembler::Status::Incomplete);

  result = reassembler.accept(makeFragment(7, 12, 1, 3, "bb"));
  EXPECT_EQ(result.status, UdpFragmentReassembler::Status::Dropped);
  EXPECT_FALSE(result.dropReason.isEmpty());
}
