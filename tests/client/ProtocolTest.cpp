#include "Protocol.h"

#include <gtest/gtest.h>

TEST(ProtocolTest, TcpMessage_RoundTripsFramedPayload) {
  QByteArray buffer =
      Protocol::encodeTcpMessage(ControlMessage::JoinCall, "payload");

  Protocol::TcpMessage message;
  EXPECT_EQ(Protocol::takeTcpMessage(buffer, message),
            Protocol::DecodeResult::Success);
  EXPECT_EQ(message.type, ControlMessage::JoinCall);
  EXPECT_EQ(message.payload, QByteArray("payload"));
  EXPECT_TRUE(buffer.isEmpty());
}

TEST(ProtocolTest, TcpMessage_WaitsForIncompletePayload) {
  QByteArray buffer = Protocol::encodeTcpMessage(ControlMessage::Ping, "abc");
  buffer.chop(1);

  Protocol::TcpMessage message;
  EXPECT_EQ(Protocol::takeTcpMessage(buffer, message),
            Protocol::DecodeResult::Incomplete);
}

TEST(ProtocolTest, UdpPacket_RoundTripsHeaderAndPayload) {
  Protocol::UdpPacket packet;
  packet.header.packetType = Protocol::UdpPacketType::VideoPacket;
  packet.header.clientId = 42;
  packet.header.sequenceNumber = 7;
  packet.header.timestampMs = 123456;
  packet.header.frameId = 9;
  packet.header.fragmentIndex = 1;
  packet.header.fragmentCount = 3;
  packet.payload = "video";

  Protocol::UdpPacket decoded;
  ASSERT_TRUE(Protocol::decodeUdpPacket(Protocol::encodeUdpPacket(packet),
                                        decoded));
  EXPECT_EQ(decoded.header.packetType, packet.header.packetType);
  EXPECT_EQ(decoded.header.clientId, 42u);
  EXPECT_EQ(decoded.header.sequenceNumber, 7u);
  EXPECT_EQ(decoded.header.timestampMs, 123456u);
  EXPECT_EQ(decoded.header.frameId, 9u);
  EXPECT_EQ(decoded.header.fragmentIndex, 1u);
  EXPECT_EQ(decoded.header.fragmentCount, 3u);
  EXPECT_EQ(decoded.payload, QByteArray("video"));
}

TEST(ProtocolTest, UdpPacket_RejectsMalformedPacket) {
  Protocol::UdpPacket decoded;
  EXPECT_FALSE(Protocol::decodeUdpPacket(QByteArray("bad"), decoded));
}
