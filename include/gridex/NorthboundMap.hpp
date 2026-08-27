#pragma once

#include <cstdint>

namespace gridex::northbound {

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
inline constexpr std::uint16_t BatteryAlarmBits = 8;
inline constexpr std::uint16_t QualityBits = 9;
inline constexpr std::uint16_t EdgeState = 10;
inline constexpr std::uint16_t AppliedPowerKwX10 = 11;
}  // namespace input

}  // namespace gridex::northbound
