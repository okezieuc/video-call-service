#include "UdpRelay.h"

#include <QHostAddress>
#include <gtest/gtest.h>

#include <algorithm>

namespace {

bool containsClientId(const QVector<std::uint32_t> &clientIds,
                      std::uint32_t clientId) {
  return std::find(clientIds.cbegin(), clientIds.cend(), clientId) !=
         clientIds.cend();
}

} // namespace

TEST(UdpRelayTest, ForwardingClientIdsExcludeSender) {
  UdpRelay relay(0);
  relay.registerClient(1);
  relay.registerClient(2);
  relay.registerClient(3);

  ASSERT_TRUE(
      relay.registerEndpointForClient(1, QHostAddress::LocalHost, 10001));
  ASSERT_TRUE(
      relay.registerEndpointForClient(2, QHostAddress::LocalHost, 10002));
  ASSERT_TRUE(
      relay.registerEndpointForClient(3, QHostAddress::LocalHost, 10003));

  const auto targets = relay.forwardingClientIds(1);
  EXPECT_FALSE(containsClientId(targets, 1));
  EXPECT_TRUE(containsClientId(targets, 2));
  EXPECT_TRUE(containsClientId(targets, 3));
  EXPECT_EQ(targets.size(), 2);
}

TEST(UdpRelayTest, ForwardingClientIdsSkipUnregisteredClients) {
  UdpRelay relay(0);
  relay.registerClient(1);
  relay.registerClient(2);

  ASSERT_TRUE(
      relay.registerEndpointForClient(1, QHostAddress::LocalHost, 10001));

  const auto targets = relay.forwardingClientIds(1);
  EXPECT_TRUE(targets.isEmpty());
}

TEST(UdpRelayTest, RegisterEndpointRejectsUnknownClients) {
  UdpRelay relay(0);
  EXPECT_FALSE(
      relay.registerEndpointForClient(99, QHostAddress::LocalHost, 10099));
}
