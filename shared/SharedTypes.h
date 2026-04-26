#pragma once

#include <cstdint>

namespace ControlMessage {
constexpr std::uint8_t Ping = 0x01;
constexpr std::uint8_t Pong = 0x02;
constexpr std::uint8_t EndCall = 0x03;
constexpr std::uint8_t CameraOff = 0x04;
constexpr std::uint8_t CameraOn = 0x05;
constexpr std::uint8_t Heartbeat = 0x06;
constexpr std::uint8_t JoinCall = 0x07;
constexpr std::uint8_t JoinAccepted = 0x08;
constexpr std::uint8_t UdpRegistered = 0x09;
constexpr std::uint8_t LeaveCall = 0x0a;
} // namespace ControlMessage
