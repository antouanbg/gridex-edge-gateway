#include "gridex/rockpie/NorthboundRegisterBank.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace gridex::rockpie {
namespace {

std::uint16_t encodeSignedX10(double value) {
    const auto scaled = std::clamp(
        std::round(value * 10.0),
        static_cast<double>(std::numeric_limits<std::int16_t>::min()),
        static_cast<double>(std::numeric_limits<std::int16_t>::max())
    );
    return static_cast<std::uint16_t>(static_cast<std::int16_t>(scaled));
}

std::uint16_t encodeUnsigned(double value, double scale) {
    if (!std::isfinite(value)) {
        return 0;
    }
    return static_cast<std::uint16_t>(std::clamp(
        std::round(value * scale),
        0.0,
        static_cast<double>(std::numeric_limits<std::uint16_t>::max())
    ));
}

std::uint32_t encodeUnsigned32(double value, double scale) {
    if (!std::isfinite(value)) return 0U;
    const auto scaled = std::clamp(
        std::round(value * scale),
        0.0,
        static_cast<double>(std::numeric_limits<std::uint32_t>::max())
    );
    return static_cast<std::uint32_t>(scaled);
}

template <std::size_t Size>
std::optional<std::vector<std::uint16_t>> readRange(
    const std::array<std::uint16_t, Size>& registers,
    std::uint16_t start,
    std::uint16_t count
) {
    const auto end = static_cast<std::size_t>(start) + count;
    if (count == 0U || end > registers.size()) {
        return std::nullopt;
    }
    return std::vector<std::uint16_t>(
        registers.begin() + start,
        registers.begin() + end
    );
}

}  // namespace

std::optional<std::vector<std::uint16_t>>
NorthboundRegisterBank::readHolding(
    std::uint16_t start,
    std::uint16_t count
) const {
    std::scoped_lock lock(mutex_);
    return readRange(holding_, start, count);
}

std::optional<std::vector<std::uint16_t>>
NorthboundRegisterBank::readInput(
    std::uint16_t start,
    std::uint16_t count
) const {
    std::scoped_lock lock(mutex_);
    return readRange(input_, start, count);
}

bool NorthboundRegisterBank::writeHolding(
    std::uint16_t start,
    std::span<const std::uint16_t> values
) {
    const auto end = static_cast<std::size_t>(start) + values.size();
    if (values.empty() || end > holding_.size()) {
        return false;
    }
    std::scoped_lock lock(mutex_);
    std::copy(values.begin(), values.end(), holding_.begin() + start);
    return true;
}

std::optional<NorthboundCommand> NorthboundRegisterBank::takeCommand() {
    std::scoped_lock lock(mutex_);
    const auto sequence = holding_[northbound::holding::CommandSequence];
    const auto heartbeat = holding_[northbound::holding::EmsHeartbeat];
    if (commandObserved_ && sequence == lastSequence_ &&
        heartbeat == lastHeartbeat_) {
        return std::nullopt;
    }

    commandObserved_ = true;
    lastSequence_ = sequence;
    lastHeartbeat_ = heartbeat;
    const auto rawPower = static_cast<std::int16_t>(
        holding_[northbound::holding::RequestedPowerKwX10]
    );
    const bool enabled = holding_[northbound::holding::CommandEnable] == 1U;
    return NorthboundCommand{
        .sequence = sequence,
        .heartbeat = heartbeat,
        .requestedPowerKw = enabled
            ? static_cast<double>(rawPower) / 10.0
            : 0.0,
        .enabled = enabled,
    };
}

std::optional<NorthboundOperatorCommand>
NorthboundRegisterBank::takeOperatorCommand() {
    std::scoped_lock lock(mutex_);
    const auto sequence = holding_[northbound::holding::OperatorSequence];
    if (operatorCommandObserved_ && sequence == lastOperatorSequence_) {
        return std::nullopt;
    }
    operatorCommandObserved_ = true;
    lastOperatorSequence_ = sequence;
    return NorthboundOperatorCommand{
        .sequence = sequence,
        .actionMask = holding_[northbound::holding::OperatorActionMask],
        .requestedRunState =
            holding_[northbound::holding::RequestedRunState],
        .requestedReactivePowerKvar = static_cast<double>(
            static_cast<std::int16_t>(
                holding_[northbound::holding::RequestedReactivePowerKvarX10]
            )
        ) / 10.0,
        .requestedSocUpperPct = static_cast<double>(holding_[
            northbound::holding::RequestedSocUpperPct
        ]),
        .requestedSocLowerPct = static_cast<double>(holding_[
            northbound::holding::RequestedSocLowerPct
        ]),
        .authorized = holding_[northbound::holding::OperatorApplyKey] ==
                      northbound::OperatorApplyKeyValue,
    };
}

void NorthboundRegisterBank::publishOperatorResult(
    std::uint16_t sequence,
    std::uint16_t result
) {
    std::scoped_lock lock(mutex_);
    input_[northbound::input::LastOperatorSequence] = sequence;
    input_[northbound::input::LastOperatorResult] = result;
}

void NorthboundRegisterBank::publish(
    const ControllerSnapshot& snapshot,
    const ConfiguredPowerLimit& configuredLimit
) {
    std::scoped_lock lock(mutex_);
    input_[northbound::input::ActualPowerKwX10] =
        encodeSignedX10(snapshot.battery.actualPowerKw);
    input_[northbound::input::SocPctX10] =
        encodeUnsigned(snapshot.battery.socPct, 10.0);
    input_[northbound::input::SohPct] =
        encodeUnsigned(snapshot.battery.sohPct, 1.0);
    input_[northbound::input::VoltageVX10] =
        encodeUnsigned(snapshot.battery.voltageV, 10.0);
    input_[northbound::input::CurrentAX10] =
        encodeSignedX10(snapshot.battery.currentA);
    input_[northbound::input::MaxChargeKwX10] =
        encodeUnsigned(snapshot.battery.maxChargeKw, 10.0);
    input_[northbound::input::MaxDischargeKwX10] =
        encodeUnsigned(snapshot.battery.maxDischargeKw, 10.0);
    input_[northbound::input::BatteryStatus] = snapshot.battery.statusCode;
    input_[northbound::input::BatterySystemFlags] =
        snapshot.battery.bmsSystemFlags;

    std::uint16_t qualityBits = 0U;
    if (snapshot.battery.quality == Quality::Good) qualityBits |= 1U << 0U;
    if (snapshot.battery.limitsValid) qualityBits |= 1U << 1U;
    if (snapshot.emsConnected) qualityBits |= 1U << 2U;
    if (snapshot.heartbeatOk) qualityBits |= 1U << 3U;
    if (!snapshot.battery.pcsFault && !snapshot.battery.bmsFault) {
        qualityBits |= 1U << 4U;
    }
    if (snapshot.battery.controlReady) qualityBits |= 1U << 5U;
    if (snapshot.battery.extendedTelemetryValid) qualityBits |= 1U << 6U;
    input_[northbound::input::QualityBits] = qualityBits;
    input_[northbound::input::EdgeState] =
        static_cast<std::uint16_t>(snapshot.state);
    input_[northbound::input::AppliedPowerKwX10] =
        encodeSignedX10(snapshot.command.appliedPowerKw);
    input_[northbound::input::ControlReady] =
        snapshot.battery.controlReady ? 1U : 0U;
    input_[northbound::input::ConfiguredMaxChargeKwX10] = encodeUnsigned(
        configuredLimit.maxChargeKw.value_or(0.0),
        10.0
    );
    input_[northbound::input::ConfiguredMaxDischargeKwX10] = encodeUnsigned(
        configuredLimit.maxDischargeKw.value_or(0.0),
        10.0
    );
    input_[northbound::input::DcPowerKwX10] =
        encodeSignedX10(snapshot.battery.dcPowerKw);
    input_[northbound::input::CommandedPowerKwX10] =
        encodeSignedX10(snapshot.battery.commandedPowerKw);
    input_[northbound::input::PcsStatus] = snapshot.battery.pcsStatusCode;
    input_[northbound::input::ReactivePowerKvarX10] =
        encodeSignedX10(snapshot.battery.reactivePowerKvar);
    const auto accumulatedCharge = encodeUnsigned32(
        snapshot.battery.accumulatedChargeKwh,
        10.0
    );
    const auto accumulatedDischarge = encodeUnsigned32(
        snapshot.battery.accumulatedDischargeKwh,
        10.0
    );
    input_[northbound::input::AccumulatedChargeKwhX10High] =
        static_cast<std::uint16_t>(accumulatedCharge >> 16U);
    input_[northbound::input::AccumulatedChargeKwhX10Low] =
        static_cast<std::uint16_t>(accumulatedCharge);
    input_[northbound::input::AccumulatedDischargeKwhX10High] =
        static_cast<std::uint16_t>(accumulatedDischarge >> 16U);
    input_[northbound::input::AccumulatedDischargeKwhX10Low] =
        static_cast<std::uint16_t>(accumulatedDischarge);
    input_[northbound::input::DailyChargeKwhX10] =
        encodeUnsigned(snapshot.battery.dailyChargeKwh, 10.0);
    input_[northbound::input::DailyDischargeKwhX10] =
        encodeUnsigned(snapshot.battery.dailyDischargeKwh, 10.0);
    std::uint16_t alarmBits = 0U;
    if (snapshot.battery.pcsWarning) alarmBits |= 1U << 0U;
    if (snapshot.battery.bmsAlarm) alarmBits |= 1U << 1U;
    input_[northbound::input::AlarmBits] = alarmBits;
    input_[northbound::input::FrequencyHzX100] =
        encodeUnsigned(snapshot.battery.frequencyHz, 100.0);
    input_[northbound::input::SocUpperLimitPct] =
        encodeUnsigned(snapshot.battery.socUpperLimitPct, 1.0);
    input_[northbound::input::SocLowerLimitPct] =
        encodeUnsigned(snapshot.battery.socLowerLimitPct, 1.0);
    input_[northbound::input::ExtendedTelemetryValid] =
        snapshot.battery.extendedTelemetryValid ? 1U : 0U;
}

void NorthboundRegisterBank::publishNode(
    std::size_t slot,
    const MbusNodeTelemetry& sample
) {
    if (slot >= northbound::NodeSlotCount) return;
    const auto base = static_cast<std::size_t>(northbound::NodeSlotBase) +
                      slot * northbound::NodeSlotStride;
    std::scoped_lock lock(mutex_);
    input_[base + northbound::node::Online] = sample.online ? 1U : 0U;
    input_[base + northbound::node::Address] = sample.address;
    input_[base + northbound::node::NodeType] = sample.nodeType;
    input_[base + northbound::node::NodeState] = sample.nodeState;
    input_[base + northbound::node::DriverId] = sample.driverId;
    input_[base + northbound::node::Quality] = sample.quality;
    input_[base + northbound::node::Heartbeat] = sample.heartbeat;
    input_[base + northbound::node::ActualPowerKwX10] =
        encodeSignedX10(sample.actualPowerKw);
    input_[base + northbound::node::EnergyWhHigh] =
        static_cast<std::uint16_t>(sample.energyWh >> 16U);
    input_[base + northbound::node::EnergyWhLow] =
        static_cast<std::uint16_t>(sample.energyWh);
    input_[base + northbound::node::DeviceState] = sample.deviceState;
    input_[base + northbound::node::AlarmBits] = sample.alarmBits;
    input_[base + northbound::node::CloudConnected] =
        sample.cloudConnected ? 1U : 0U;
    const auto age = sample.online
        ? std::chrono::duration_cast<std::chrono::seconds>(
              std::chrono::steady_clock::now() - sample.lastSeen
          ).count()
        : std::numeric_limits<std::uint16_t>::max();
    input_[base + northbound::node::AgeSeconds] = static_cast<std::uint16_t>(
        std::clamp<std::int64_t>(age, 0, std::numeric_limits<std::uint16_t>::max())
    );
}

}  // namespace gridex::rockpie
