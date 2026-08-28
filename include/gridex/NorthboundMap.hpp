#pragma once

#include <cstdint>

namespace gridex::northbound {

inline constexpr std::uint16_t HoldingRegisterCount = 4;
inline constexpr std::uint16_t NodeSlotBase = 0x0100;
inline constexpr std::uint16_t NodeSlotStride = 16;
inline constexpr std::uint16_t NodeSlotCount = 32;
inline constexpr std::uint16_t InputRegisterCount =
    NodeSlotBase + NodeSlotStride * NodeSlotCount;

// PDU zero-based addresses exposed by the Edge Modbus TCP server on port 1502.
namespace holding {
inline constexpr std::uint16_t CommandSequence = 0;
inline constexpr std::uint16_t RequestedPowerKwX10 = 1;
inline constexpr std::uint16_t CommandEnable = 2;
inline constexpr std::uint16_t EmsHeartbeat = 3;
}  // namespace holding

namespace input {
inline constexpr std::uint16_t ActualPowerKwX10 = 0;
inline constexpr std::uint16_t SocPctX10 = 1;
inline constexpr std::uint16_t SohPct = 2;
inline constexpr std::uint16_t VoltageVX10 = 3;
inline constexpr std::uint16_t CurrentAX10 = 4;
inline constexpr std::uint16_t MaxChargeKwX10 = 5;
inline constexpr std::uint16_t MaxDischargeKwX10 = 6;
inline constexpr std::uint16_t BatteryStatus = 7;
inline constexpr std::uint16_t BatterySystemFlags = 8;
inline constexpr std::uint16_t QualityBits = 9;
inline constexpr std::uint16_t EdgeState = 10;
inline constexpr std::uint16_t AppliedPowerKwX10 = 11;
inline constexpr std::uint16_t ControlReady = 12;
inline constexpr std::uint16_t ConfiguredMaxChargeKwX10 = 13;
inline constexpr std::uint16_t ConfiguredMaxDischargeKwX10 = 14;
}  // namespace input

namespace node {
inline constexpr std::uint16_t Online = 0;
inline constexpr std::uint16_t Address = 1;
inline constexpr std::uint16_t NodeType = 2;
inline constexpr std::uint16_t NodeState = 3;
inline constexpr std::uint16_t DriverId = 4;
inline constexpr std::uint16_t Quality = 5;
inline constexpr std::uint16_t Heartbeat = 6;
inline constexpr std::uint16_t ActualPowerKwX10 = 7;
inline constexpr std::uint16_t EnergyWhHigh = 8;
inline constexpr std::uint16_t EnergyWhLow = 9;
inline constexpr std::uint16_t DeviceState = 10;
inline constexpr std::uint16_t AlarmBits = 11;
inline constexpr std::uint16_t AgeSeconds = 12;
inline constexpr std::uint16_t CloudConnected = 13;
}  // namespace node

}  // namespace gridex::northbound
