#pragma once

#include "gridex/SafetyEnvelope.hpp"
#include "gridex/SunStoragePro261Driver.hpp"
#include "gridex/Types.hpp"

#include <chrono>
#include <optional>

namespace gridex {

struct ControllerConfig {
    std::chrono::seconds emsTimeout{15};
    std::chrono::seconds heartbeatRefresh{35};
    std::uint16_t heartbeatTimeoutSeconds{60};
};

struct ControllerSnapshot {
    EdgeState state{EdgeState::Boot};
    BatteryTelemetry battery;
    SafetyResult command;
    bool emsConnected{false};
    bool heartbeatOk{false};
};

class EdgeController {
public:
    EdgeController(
        SunStoragePro261Driver& driver,
        SafetyEnvelope safety,
        ControllerConfig config = {}
    );

    void receiveCommand(double requestedPowerKw, SiteLimit site, SteadyTime now);
    [[nodiscard]] ControllerSnapshot tick(SteadyTime now);

private:
    SunStoragePro261Driver& driver_;
    SafetyEnvelope safety_;
    ControllerConfig config_;
    double requestedPowerKw_{0.0};
    SiteLimit site_;
    std::optional<SteadyTime> lastCommandAt_;
    std::optional<SteadyTime> lastHeartbeatAt_;
};

}  // namespace gridex

