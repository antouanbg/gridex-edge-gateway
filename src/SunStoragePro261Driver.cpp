#include "gridex/SunStoragePro261Driver.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace gridex {

SunStoragePro261Driver::SunStoragePro261Driver(
    IModbusClient& client,
    CommissioningApproval approval
)
    : client_(client), approval_(approval) {}

double SunStoragePro261Driver::decodeSigned(
    std::uint16_t raw,
    double scale
) {
    return static_cast<double>(static_cast<std::int16_t>(raw)) / scale;
}

std::uint16_t SunStoragePro261Driver::encodeSigned(
    double value,
    double scale
) {
    const double scaled = std::round(value * scale);
    const double bounded = std::clamp(
        scaled,
        static_cast<double>(std::numeric_limits<std::int16_t>::min()),
        static_cast<double>(std::numeric_limits<std::int16_t>::max())
    );
    return static_cast<std::uint16_t>(static_cast<std::int16_t>(bounded));
}

double SunStoragePro261Driver::decodeSigned32(
    std::uint16_t high,
    std::uint16_t low,
    double scale
) {
    const auto raw = (static_cast<std::uint32_t>(high) << 16U) | low;
    return static_cast<double>(static_cast<std::int32_t>(raw)) / scale;
}

BatteryTelemetry SunStoragePro261Driver::poll() {
    BatteryTelemetry value;

    const auto pcsFault = client_.readCoil(Registers::PcsFault);
    const auto pcsOffGrid = client_.readCoil(Registers::PcsOffGrid);
    const auto pcsWarning = client_.readCoil(Registers::PcsWarning);
    const auto pcsCommunicationFault =
        client_.readCoil(Registers::PcsCommunicationFault);
    const auto bmsAlarm = client_.readCoil(Registers::BmsAlarm);
    const auto bmsFault = client_.readCoil(Registers::BmsFault);
    const auto bmsCommunicationFault =
        client_.readCoil(Registers::BmsCommunicationFault);
    const auto pcsGridMode = client_.readHolding(Registers::PcsGridMode);
    const auto pcsWorkMode = client_.readHolding(Registers::PcsWorkMode);
    const auto pcsPowerOn = client_.readHolding(Registers::PcsPowerOn);
    const auto socUpperLimit =
        client_.readHolding(Registers::PcsSocUpperLimit);
    const auto socLowerLimit =
        client_.readHolding(Registers::PcsSocLowerLimit);
    const auto actualPower = client_.readInput(Registers::PcsActivePower);
    const auto reactivePower = client_.readInput(Registers::PcsReactivePower);
    const auto dcPower = client_.readInput(Registers::PcsDcPower);
    const auto frequency = client_.readInput(Registers::PcsFrequency);
    const auto commandedPower =
        client_.readInput(Registers::PcsCommandedPower);
    const auto pcsStatus = client_.readInput(Registers::PcsStatus);
    const auto current = client_.readInput(Registers::BmsCurrent);
    const auto soc = client_.readInput(Registers::BmsSoc);
    const auto soh = client_.readInput(Registers::BmsSoh);
    const auto voltage = client_.readInput(Registers::BmsVoltage);
    const auto status = client_.readInput(Registers::BmsStatus);
    const auto accumulatedChargeHigh =
        client_.readInput(Registers::BmsAccumulatedChargeEnergy);
    const auto accumulatedChargeLow =
        client_.readInput(Registers::BmsAccumulatedChargeEnergy + 1U);
    const auto accumulatedDischargeHigh =
        client_.readInput(Registers::BmsAccumulatedDischargeEnergy);
    const auto accumulatedDischargeLow =
        client_.readInput(Registers::BmsAccumulatedDischargeEnergy + 1U);
    const auto systemFlags = client_.readInput(Registers::BmsSystemFlags);
    const auto maxCharge = client_.readInput(Registers::BmsMaxChargePower);
    const auto maxDischarge =
        client_.readInput(Registers::BmsMaxDischargePower);
    const auto dailyCharge =
        client_.readInput(Registers::BmsDailyChargeEnergy);
    const auto dailyDischarge =
        client_.readInput(Registers::BmsDailyDischargeEnergy);

    if (!pcsFault || !pcsOffGrid || !pcsCommunicationFault || !bmsFault ||
        !bmsCommunicationFault || !pcsGridMode || !pcsWorkMode ||
        !pcsPowerOn || !actualPower || !current || !soc || !soh ||
        !voltage || !status || !systemFlags || !maxCharge || !maxDischarge) {
        value.quality = Quality::Invalid;
        value.limitsValid = false;
        controlReady_ = false;
        limitsValid_ = false;
        telemetryValid_ = false;
        faultsClear_ = false;
        gridModeReady_ = false;
        workModeReady_ = false;
        return value;
    }

    // Canonical GrideX sign: positive = discharge, negative = charge.
    value.actualPowerKw = decodeSigned(*actualPower, 10.0);
    value.commandedPowerKw = commandedPower
        ? decodeSigned(*commandedPower, 10.0)
        : 0.0;
    value.dcPowerKw = dcPower ? decodeSigned(*dcPower, 10.0) : 0.0;
    value.reactivePowerKvar = reactivePower
        ? decodeSigned(*reactivePower, 10.0)
        : 0.0;
    value.frequencyHz = frequency
        ? decodeSigned(*frequency, 100.0)
        : 0.0;
    value.currentA = decodeSigned(*current, 10.0);
    value.socPct = decodeSigned(*soc, 10.0);
    value.sohPct = decodeSigned(*soh, 1.0);
    value.voltageV = decodeSigned(*voltage, 10.0);
    value.statusCode = *status;
    value.pcsStatusCode = pcsStatus.value_or(0U);
    value.bmsSystemFlags = *systemFlags;
    value.pcsPowerOn = *pcsPowerOn == 1U;
    value.pcsGridTied = *pcsGridMode == 0U && !*pcsOffGrid;
    value.pcsCurrentSourceMode = *pcsWorkMode == 1U;
    value.pcsFault = *pcsFault;
    value.pcsCommunicationFault = *pcsCommunicationFault;
    value.bmsFault = *bmsFault;
    value.bmsCommunicationFault = *bmsCommunicationFault;
    value.bmsAlarm = bmsAlarm.value_or(true);
    value.pcsWarning = pcsWarning.value_or(true);
    value.socUpperLimitPct = socUpperLimit
        ? decodeSigned(*socUpperLimit, 1.0)
        : 0.0;
    value.socLowerLimitPct = socLowerLimit
        ? decodeSigned(*socLowerLimit, 1.0)
        : 0.0;
    if (accumulatedChargeHigh && accumulatedChargeLow) {
        value.accumulatedChargeKwh = decodeSigned32(
            approval_.int32HighWordFirst
                ? *accumulatedChargeHigh
                : *accumulatedChargeLow,
            approval_.int32HighWordFirst
                ? *accumulatedChargeLow
                : *accumulatedChargeHigh,
            10.0
        );
    }
    if (accumulatedDischargeHigh && accumulatedDischargeLow) {
        value.accumulatedDischargeKwh = decodeSigned32(
            approval_.int32HighWordFirst
                ? *accumulatedDischargeHigh
                : *accumulatedDischargeLow,
            approval_.int32HighWordFirst
                ? *accumulatedDischargeLow
                : *accumulatedDischargeHigh,
            10.0
        );
    }
    value.dailyChargeKwh = dailyCharge
        ? decodeSigned(*dailyCharge, 10.0)
        : 0.0;
    value.dailyDischargeKwh = dailyDischarge
        ? decodeSigned(*dailyDischarge, 10.0)
        : 0.0;
    value.extendedTelemetryValid =
        pcsWarning && bmsAlarm && socUpperLimit && socLowerLimit &&
        reactivePower && dcPower && frequency && commandedPower && pcsStatus &&
        accumulatedChargeHigh && accumulatedChargeLow &&
        accumulatedDischargeHigh && accumulatedDischargeLow &&
        dailyCharge && dailyDischarge && approval_.int32WordOrderConfirmed;
    const double decodedMaxChargeKw = decodeSigned(*maxCharge, 10.0);
    const double decodedMaxDischargeKw = decodeSigned(*maxDischarge, 10.0);
    value.maxChargeKw = std::max(0.0, decodedMaxChargeKw);
    value.maxDischargeKw = std::max(0.0, decodedMaxDischargeKw);
    value.limitsValid =
        decodedMaxChargeKw >= 0.0 && decodedMaxDischargeKw >= 0.0 &&
        value.socPct >= 0.0 && value.socPct <= 100.0;
    value.controlReady =
        value.pcsPowerOn && value.pcsGridTied &&
        value.pcsCurrentSourceMode && !value.pcsFault &&
        !value.pcsCommunicationFault && !value.bmsFault &&
        !value.bmsCommunicationFault;
    controlReady_ = value.controlReady;
    telemetryValid_ = true;
    faultsClear_ = !value.pcsFault && !value.pcsCommunicationFault &&
                   !value.bmsFault && !value.bmsCommunicationFault;
    gridModeReady_ = value.pcsGridTied;
    workModeReady_ = value.pcsCurrentSourceMode;
    limitsValid_ = value.limitsValid;
    maxChargeKw_ = value.maxChargeKw;
    maxDischargeKw_ = value.maxDischargeKw;
    bmsSystemFlags_ = value.bmsSystemFlags;
    value.quality = value.limitsValid ? Quality::Good : Quality::Invalid;
    return value;
}

bool SunStoragePro261Driver::refreshHeartbeat(
    std::uint16_t timeoutSeconds
) {
    if (!approval_.writesEnabled()) {
        return false;
    }
    return client_.writeHolding(Registers::HeartbeatEnable, 1U) &&
           client_.writeHolding(Registers::HeartbeatSeconds, timeoutSeconds);
}

bool SunStoragePro261Driver::writePowerSetpointKw(
    double canonicalPowerKw
) {
    if (!approval_.writesEnabled() || !controlReady_ || !limitsValid_ ||
        !std::isfinite(canonicalPowerKw)) {
        return false;
    }
    constexpr double toleranceKw = 0.001;
    const bool chargeProhibited = (bmsSystemFlags_ & 0x0001U) != 0U;
    const bool dischargeProhibited = (bmsSystemFlags_ & 0x0002U) != 0U;
    if ((canonicalPowerKw < -toleranceKw &&
         (chargeProhibited || -canonicalPowerKw > maxChargeKw_ + toleranceKw)) ||
        (canonicalPowerKw > toleranceKw &&
         (dischargeProhibited ||
          canonicalPowerKw > maxDischargeKw_ + toleranceKw))) {
        return false;
    }
    return client_.writeHolding(
        Registers::PcsPowerCommand,
        encodeSigned(canonicalPowerKw, 10.0)
    );
}

bool SunStoragePro261Driver::writeReactivePowerSetpointKvar(
    double reactivePowerKvar
) {
    if (!approval_.writesEnabled() || !controlReady_ ||
        !std::isfinite(reactivePowerKvar)) {
        return false;
    }
    return client_.writeHolding(
        Registers::PcsReactivePowerCommand,
        encodeSigned(reactivePowerKvar, 10.0)
    );
}

bool SunStoragePro261Driver::writePowerState(bool powerOn) {
    if (!approval_.writesEnabled() || !telemetryValid_) {
        return false;
    }
    if (powerOn) {
        if (!faultsClear_ || !gridModeReady_ || !workModeReady_) {
            return false;
        }
        return client_.writeHolding(Registers::PcsPowerOn, 1U);
    }

    const bool zeroWritten = client_.writeHolding(
        Registers::PcsPowerCommand,
        encodeSigned(0.0, 10.0)
    );
    const bool stopped = client_.writeHolding(Registers::PcsPowerOn, 0U);
    if (stopped) {
        controlReady_ = false;
    }
    return zeroWritten && stopped;
}

bool SunStoragePro261Driver::writeSocLimitsPct(
    double lowerPct,
    double upperPct
) {
    if (!approval_.writesEnabled() || !telemetryValid_ ||
        !std::isfinite(lowerPct) || !std::isfinite(upperPct) ||
        lowerPct < 0.0 || upperPct > 100.0 || lowerPct >= upperPct) {
        return false;
    }
    const auto currentUpper = client_.readHolding(Registers::PcsSocUpperLimit);
    const auto currentLower = client_.readHolding(Registers::PcsSocLowerLimit);
    if (!currentUpper || !currentLower) {
        return false;
    }
    const auto requestedUpper = encodeSigned(upperPct, 1.0);
    const auto requestedLower = encodeSigned(lowerPct, 1.0);
    const auto oldUpper = decodeSigned(*currentUpper, 1.0);
    const auto oldLower = decodeSigned(*currentLower, 1.0);

    // Widen first, then tighten, so a partial write cannot invert the window.
    if (upperPct > oldUpper &&
        !client_.writeHolding(Registers::PcsSocUpperLimit, requestedUpper)) {
        return false;
    }
    if (lowerPct < oldLower &&
        !client_.writeHolding(Registers::PcsSocLowerLimit, requestedLower)) {
        return false;
    }
    if (upperPct <= oldUpper &&
        !client_.writeHolding(Registers::PcsSocUpperLimit, requestedUpper)) {
        return false;
    }
    if (lowerPct >= oldLower &&
        !client_.writeHolding(Registers::PcsSocLowerLimit, requestedLower)) {
        return false;
    }
    return true;
}

bool SunStoragePro261Driver::writesEnabled() const {
    return approval_.writesEnabled();
}

bool SunStoragePro261Driver::controlReady() const {
    return controlReady_;
}

}  // namespace gridex
