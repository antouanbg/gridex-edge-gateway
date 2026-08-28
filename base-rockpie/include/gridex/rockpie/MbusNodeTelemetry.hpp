#pragma once

#include <chrono>
#include <cstdint>

namespace gridex::rockpie {

struct MbusNodeTelemetry {
    std::uint8_t address{};
    std::uint16_t nodeType{};
    std::uint16_t nodeState{};
    std::uint16_t driverId{};
    std::uint16_t quality{};
    std::uint16_t heartbeat{};
    double actualPowerKw{};
    std::uint32_t energyWh{};
    std::uint16_t deviceState{};
    std::uint16_t alarmBits{};
    bool cloudConnected{false};
    bool online{false};
    std::chrono::steady_clock::time_point lastSeen{};
};

}  // namespace gridex::rockpie
