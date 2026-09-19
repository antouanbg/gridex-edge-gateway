#pragma once

#include <chrono>
#include <cstdint>

namespace gridex::rockpie {

enum class NodePollStatus : std::uint16_t {
    Unknown = 0,
    Online = 1,
    TransportFailure = 2,
    IdentityFailure = 3,
    TelemetryFailure = 4,
};

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
    NodePollStatus pollStatus{NodePollStatus::Unknown};
    std::uint32_t consecutiveFailures{0};
    std::uint16_t ethernetStatus{0};
    std::uint16_t modbusTcpStatus{0};
    std::uint16_t driverReady{0};
    std::uint16_t deviceBusStatus{0};
    std::uint16_t watchdogStatus{0};
    std::uint16_t recoveryCount{0};
    std::uint16_t ethernetRecoveryCount{0};
    std::uint16_t busRecoveryCount{0};
    std::uint16_t lastError{0};
    std::chrono::steady_clock::time_point lastSeen{};
    std::chrono::system_clock::time_point lastSuccessfulContact{};
};

}  // namespace gridex::rockpie
