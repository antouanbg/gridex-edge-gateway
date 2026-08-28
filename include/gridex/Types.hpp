#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

namespace gridex {

enum class Quality {
    Invalid,
    Good,
    Stale,
    Fault,
};

enum class EdgeState : std::uint16_t {
    Boot = 0,
    WaitingForComms = 1,
    WaitingForBmsLimits = 2,
    Ready = 3,
    SafeMode = 4,
    Fault = 5,
    CommissioningLocked = 6,
    WaitingForPcsReady = 7,
};

struct BatteryTelemetry {
    double actualPowerKw{};
    double socPct{};
    double sohPct{};
    double voltageV{};
    double currentA{};
    double maxChargeKw{};
    double maxDischargeKw{};
    std::uint16_t statusCode{};
    std::uint16_t bmsSystemFlags{};
    Quality quality{Quality::Invalid};
    bool limitsValid{false};
    bool pcsPowerOn{false};
    bool pcsGridTied{false};
    bool pcsCurrentSourceMode{false};
    bool pcsFault{true};
    bool pcsCommunicationFault{true};
    bool bmsFault{true};
    bool bmsCommunicationFault{true};
    bool controlReady{false};
};

struct SiteLimit {
    std::optional<double> siteLoadKw;
    std::optional<double> contractLimitKw;
};

struct ConfiguredPowerLimit {
    std::optional<double> maxChargeKw;
    std::optional<double> maxDischargeKw;
};

struct SafetyInput {
    double requestedPowerKw{};
    BatteryTelemetry battery;
    SiteLimit site;
    ConfiguredPowerLimit configuredLimit;
    bool emsConnected{false};
};

struct SafetyResult {
    double requestedPowerKw{};
    double appliedPowerKw{};
    bool clamped{false};
    std::string reason;
};

using SteadyTime = std::chrono::steady_clock::time_point;

}  // namespace gridex
