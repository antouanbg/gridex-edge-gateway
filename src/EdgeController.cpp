#include "gridex/EdgeController.hpp"

namespace gridex {

EdgeController::EdgeController(
    SunStoragePro261Driver& driver,
    SafetyEnvelope safety,
    ControllerConfig config
)
    : driver_(driver), safety_(safety), config_(config) {}

void EdgeController::receiveCommand(
    double requestedPowerKw,
    SiteLimit site,
    SteadyTime now
) {
    requestedPowerKw_ = requestedPowerKw;
    site_ = site;
    lastCommandAt_ = now;
}

ControllerSnapshot EdgeController::tick(SteadyTime now) {
    ControllerSnapshot snapshot;
    snapshot.battery = driver_.poll();
    snapshot.emsConnected =
        lastCommandAt_.has_value() &&
        (now - *lastCommandAt_) <= config_.emsTimeout;

    const bool heartbeatDue =
        !lastHeartbeatAt_.has_value() ||
        (now - *lastHeartbeatAt_) >= config_.heartbeatRefresh;
    if (heartbeatDue) {
        snapshot.heartbeatOk =
            driver_.refreshHeartbeat(config_.heartbeatTimeoutSeconds);
        if (snapshot.heartbeatOk) {
            lastHeartbeatAt_ = now;
        }
    } else {
        snapshot.heartbeatOk = true;
    }

    const bool controlAvailable =
        snapshot.emsConnected && snapshot.heartbeatOk;
    snapshot.command = safety_.apply(SafetyInput{
        .requestedPowerKw = requestedPowerKw_,
        .battery = snapshot.battery,
        .site = site_,
        .configuredLimit = config_.configuredLimit,
        .emsConnected = controlAvailable,
    });

    if (!driver_.writesEnabled()) {
        snapshot.state = EdgeState::CommissioningLocked;
        snapshot.command.appliedPowerKw = 0.0;
        snapshot.command.clamped =
            snapshot.command.clamped || requestedPowerKw_ != 0.0;
        snapshot.command.reason =
            "write path locked until commissioning is approved";
        return snapshot;
    }

    if (!snapshot.heartbeatOk) {
        snapshot.state = EdgeState::Fault;
    } else if (snapshot.battery.quality != Quality::Good) {
        snapshot.state = EdgeState::WaitingForComms;
    } else if (!snapshot.battery.limitsValid) {
        snapshot.state = EdgeState::WaitingForBmsLimits;
    } else if (!snapshot.battery.controlReady) {
        snapshot.state =
            (snapshot.battery.pcsFault || snapshot.battery.bmsFault ||
             snapshot.battery.pcsCommunicationFault ||
             snapshot.battery.bmsCommunicationFault)
                ? EdgeState::Fault
                : EdgeState::WaitingForPcsReady;
    } else if (!snapshot.emsConnected) {
        snapshot.state = EdgeState::SafeMode;
    } else {
        snapshot.state = EdgeState::Ready;
    }

    if (snapshot.battery.controlReady &&
        !driver_.writePowerSetpointKw(snapshot.command.appliedPowerKw)) {
        snapshot.state = EdgeState::Fault;
        snapshot.command.appliedPowerKw = 0.0;
        snapshot.command.clamped = true;
        snapshot.command.reason = "PCS command write failed";
    }

    return snapshot;
}

}  // namespace gridex
