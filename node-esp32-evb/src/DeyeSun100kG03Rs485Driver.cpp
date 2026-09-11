#ifdef ARDUINO

#include "gridex/drivers/DeyeSun100kG03Rs485Driver.hpp"

#include <algorithm>

namespace gridex::drivers {
namespace {
constexpr std::uint16_t DeviceType = 0x0000;
constexpr std::uint16_t ExternalControlEnable = 0x004C;
constexpr std::uint16_t ActivePowerRegulation = 0x004D;
constexpr std::uint16_t RatedPowerHigh = 0x0010;
constexpr std::uint16_t DeviceState = 0x003B;
constexpr std::uint16_t TotalEnergyHigh = 0x003F;
constexpr std::uint16_t AlarmHigh = 0x0065;
constexpr std::uint16_t OutputPowerHigh = 0x0056;
constexpr std::uint16_t DeyeStringInverter = 0x0002;
constexpr std::uint16_t NormalState = 0x0002;
}  // namespace

bool DeyeSun100kG03Rs485Driver::begin() {
    const auto deviceType = transport_.readHolding(DeviceType);
    const auto ratedHigh = transport_.readHolding(RatedPowerHigh);
    const auto ratedLow = transport_.readHolding(static_cast<std::uint16_t>(RatedPowerHigh + 1U));
    if (!deviceType || !ratedHigh || !ratedLow || *deviceType != DeyeStringInverter) {
        online_ = false;
        return false;
    }
    // The V118 profile expresses rated power as an unsigned value in 0.1 W.
    const auto ratedWatts = joinU32(*ratedHigh, *ratedLow) / 10U;
    online_ = ratedWatts >= 90000U && ratedWatts <= 110000U;
    ratedPowerKwX10_ = online_ ? static_cast<std::uint16_t>(ratedWatts / 100U) : 0U;
    return online_;
}

mbus::DriverSample DeyeSun100kG03Rs485Driver::poll() {
    mbus::DriverSample sample;
    const auto state = transport_.readHolding(DeviceState);
    const auto powerHigh = transport_.readHolding(OutputPowerHigh);
    const auto powerLow = transport_.readHolding(static_cast<std::uint16_t>(OutputPowerHigh + 1U));
    const auto energyHigh = transport_.readHolding(TotalEnergyHigh);
    const auto energyLow = transport_.readHolding(static_cast<std::uint16_t>(TotalEnergyHigh + 1U));
    const auto alarmHigh = transport_.readHolding(AlarmHigh);
    const auto alarmLow = transport_.readHolding(static_cast<std::uint16_t>(AlarmHigh + 1U));

    if (!state || !powerHigh || !powerLow || !energyHigh || !energyLow || !alarmHigh || !alarmLow) {
        online_ = false;
        sample.state = static_cast<std::uint16_t>(mbus::NodeState::Fault);
        sample.quality = 0U;
        return sample;
    }
    online_ = true;
    const auto rawPowerDeciWatts = joinU32(*powerHigh, *powerLow);
    sample.actualPowerKwX10 = static_cast<std::int16_t>(std::min<std::uint32_t>(
        rawPowerDeciWatts / 1000U, 32767U
    ));
    const auto rawEnergyDeciKwh = joinU32(*energyHigh, *energyLow);
    sample.energyWh = rawEnergyDeciKwh > 42949672U ? 4294967200U : rawEnergyDeciKwh * 100U;
    sample.state = *state;
    sample.alarms = static_cast<std::uint16_t>(*alarmHigh | *alarmLow);
    sample.quality = *state == NormalState && sample.alarms == 0U ? 1U : 2U;
    return sample;
}

bool DeyeSun100kG03Rs485Driver::applyPowerCommand(std::int16_t requestedPowerKwX10) {
    if (!config_.writesEnabled || !online_ || ratedPowerKwX10_ == 0U || requestedPowerKwX10 < 0) {
        return false;
    }
    const auto requested = static_cast<std::uint32_t>(requestedPowerKwX10);
    const auto rawTenthsPct = static_cast<std::uint32_t>(requested) * 1000U / ratedPowerKwX10_;
    const auto limitedRawTenthsPct = static_cast<std::uint16_t>(std::min<std::uint32_t>(
        rawTenthsPct, config_.maximumRegulationTenthsPct
    ));
    if (config_.requireControlEnableRegister && !transport_.writeHolding(ExternalControlEnable, 1U)) {
        return false;
    }
    if (!transport_.writeHolding(ActivePowerRegulation, limitedRawTenthsPct)) {
        return false;
    }
    const auto readBack = transport_.readHolding(ActivePowerRegulation);
    return readBack && *readBack == limitedRawTenthsPct;
}

std::uint32_t DeyeSun100kG03Rs485Driver::joinU32(std::uint16_t high, std::uint16_t low) {
    return (static_cast<std::uint32_t>(high) << 16U) | low;
}

}  // namespace gridex::drivers

#endif
