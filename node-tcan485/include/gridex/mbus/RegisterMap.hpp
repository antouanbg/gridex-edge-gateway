#pragma once

#include <cstddef>
#include <cstdint>

namespace gridex::mbus {

enum class NodeType : std::uint16_t {
    Unconfigured = 0,
    Inverter = 1,
    BatteryBms = 2,
    AllInOneBess = 3,
    Meter = 4,
    Evse = 5,
    SecondCabinet = 6,
};

enum class NodeState : std::uint16_t {
    Boot = 0,
    AwaitingAddress = 1,
    Ready = 2,
    DriverMissing = 3,
    Fault = 4,
};

namespace reg {
inline constexpr std::uint16_t Magic = 0x0000;
inline constexpr std::uint16_t MapVersion = 0x0001;
inline constexpr std::uint16_t NodeType = 0x0002;
inline constexpr std::uint16_t NodeState = 0x0003;
inline constexpr std::uint16_t NodeAddress = 0x0004;
inline constexpr std::uint16_t DriverId = 0x0005;
inline constexpr std::uint16_t Quality = 0x0006;
inline constexpr std::uint16_t Uid0 = 0x0007;
inline constexpr std::uint16_t Uid1 = 0x0008;
inline constexpr std::uint16_t Uid2 = 0x0009;
inline constexpr std::uint16_t Uid3 = 0x000A;
inline constexpr std::uint16_t UptimeLow = 0x000B;
inline constexpr std::uint16_t Heartbeat = 0x000C;

inline constexpr std::uint16_t RequestedAddress = 0x0010;
inline constexpr std::uint16_t RequestedNodeType = 0x0011;
inline constexpr std::uint16_t RequestedDriverId = 0x0012;
inline constexpr std::uint16_t ApplyConfiguration = 0x0013;
inline constexpr std::uint16_t CommandSequence = 0x0014;
inline constexpr std::uint16_t RequestedPowerKwX10 = 0x0015;
inline constexpr std::uint16_t CommandEnable = 0x0016;

inline constexpr std::uint16_t ActualPowerKwX10 = 0x0040;
inline constexpr std::uint16_t EnergyWhHigh = 0x0041;
inline constexpr std::uint16_t EnergyWhLow = 0x0042;
inline constexpr std::uint16_t DeviceState = 0x0043;
inline constexpr std::uint16_t AlarmBits = 0x0044;
inline constexpr std::uint16_t DriverSpecificStart = 0x0050;

inline constexpr std::size_t HoldingCount = 0x0080;
inline constexpr std::uint16_t MagicValue = 0x4758;  // GX
inline constexpr std::uint16_t MapVersionValue = 0x0003;
inline constexpr std::uint16_t ApplyKey = 0xA55A;
}  // namespace reg

}  // namespace gridex::mbus
