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

BatteryTelemetry SunStoragePro261Driver::poll() {
    BatteryTelemetry value;

    const auto pcsFault = client_.readCoil(Registers::PcsFault);
    const auto pcsOffGrid = client_.readCoil(Registers::PcsOffGrid);
    const auto pcsCommunicationFault =
        client_.readCoil(Registers::PcsCommunicationFault);
    const auto bmsFault = client_.readCoil(Registers::BmsFault);
    const auto bmsCommunicationFault =
        client_.readCoil(Registers::BmsCommunicationFault);
    const auto pcsGridMode = client_.readHolding(Registers::PcsGridMode);
    const auto pcsWorkMode = client_.readHolding(Registers::PcsWorkMode);
    const auto pcsPowerOn = client_.readHolding(Registers::PcsPowerOn);
    const auto actualPower = client_.readInput(Registers::PcsActivePower);
    const auto current = client_.readInput(Registers::BmsCurrent);
    const auto soc = client_.readInput(Registers::BmsSoc);
    const auto soh = client_.readInput(Registers::BmsSoh);
    const auto voltage = client_.readInput(Registers::BmsVoltage);
    const auto status = client_.readInput(Registers::BmsStatus);
    const auto systemFlags = client_.readInput(Registers::BmsSystemFlags);
    const auto maxCharge = client_.readInput(Registers::BmsMaxChargePower);
    const auto maxDischarge =
        client_.readInput(Registers::BmsMaxDischargePower);

    if (!pcsFault || !pcsOffGrid || !pcsCommunicationFault || !bmsFault ||
        !bmsCommunicationFault || !pcsGridMode || !pcsWorkMode ||
        !pcsPowerOn || !actualPower || !current || !soc || !soh ||
        !voltage || !status || !systemFlags || !maxCharge || !maxDischarge) {
        value.quality = Quality::Invalid;
        value.limitsValid = false;
        controlReady_ = false;
        return value;
    }

    // Canonical GrideX sign: positive = discharge, negative = charge.
    value.actualPowerKw = decodeSigned(*actualPower, 10.0);
    value.currentA = decodeSigned(*current, 10.0);
    value.socPct = decodeSigned(*soc, 10.0);
    value.sohPct = decodeSigned(*soh, 1.0);
    value.voltageV = decodeSigned(*voltage, 10.0);
    value.statusCode = *status;
    value.bmsSystemFlags = *systemFlags;
    value.pcsPowerOn = *pcsPowerOn == 1U;
    value.pcsGridTied = *pcsGridMode == 0U && !*pcsOffGrid;
    value.pcsCurrentSourceMode = *pcsWorkMode == 1U;
    value.pcsFault = *pcsFault;
    value.pcsCommunicationFault = *pcsCommunicationFault;
    value.bmsFault = *bmsFault;
    value.bmsCommunicationFault = *bmsCommunicationFault;
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
    if (!approval_.writesEnabled() || !controlReady_ ||
        !std::isfinite(canonicalPowerKw)) {
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

bool SunStoragePro261Driver::writesEnabled() const {
    return approval_.writesEnabled();
}

bool SunStoragePro261Driver::controlReady() const {
    return controlReady_;
}

}  // namespace gridex
