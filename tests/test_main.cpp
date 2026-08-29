#include "gridex/IModbusClient.hpp"
#include "gridex/SafetyEnvelope.hpp"
#include "gridex/SunStoragePro261Driver.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <map>
#include <optional>
#include <utility>
#include <vector>

namespace {

class FakeModbusClient final : public gridex::IModbusClient {
public:
    std::optional<std::uint16_t> readInput(std::uint16_t address) override {
        const auto it = input.find(address);
        return it == input.end() ? std::nullopt
                                 : std::optional<std::uint16_t>(it->second);
    }

    std::optional<std::uint16_t> readHolding(
        std::uint16_t address
    ) override {
        const auto it = holding.find(address);
        return it == holding.end() ? std::nullopt
                                   : std::optional<std::uint16_t>(it->second);
    }

    std::optional<bool> readCoil(std::uint16_t address) override {
        const auto it = coils.find(address);
        return it == coils.end() ? std::nullopt
                                 : std::optional<bool>(it->second);
    }

    bool writeHolding(
        std::uint16_t address,
        std::uint16_t value
    ) override {
        writeLog.emplace_back(address, value);
        holding[address] = value;
        return writesSucceed;
    }

    std::map<std::uint16_t, std::uint16_t> input;
    std::map<std::uint16_t, std::uint16_t> holding;
    std::map<std::uint16_t, bool> coils;
    std::vector<std::pair<std::uint16_t, std::uint16_t>> writeLog;
    bool writesSucceed{true};
};

bool near(double lhs, double rhs) {
    return std::abs(lhs - rhs) < 0.001;
}

gridex::BatteryTelemetry goodBattery() {
    return gridex::BatteryTelemetry{
        .actualPowerKw = 25.0,
        .socPct = 72.5,
        .sohPct = 98.0,
        .voltageV = 760.0,
        .currentA = 10.0,
        .maxChargeKw = 100.0,
        .maxDischargeKw = 120.0,
        .statusCode = 3,
        .bmsSystemFlags = 0,
        .quality = gridex::Quality::Good,
        .limitsValid = true,
        .pcsPowerOn = true,
        .pcsGridTied = true,
        .pcsCurrentSourceMode = true,
        .pcsFault = false,
        .pcsCommunicationFault = false,
        .bmsFault = false,
        .bmsCommunicationFault = false,
        .controlReady = true,
    };
}

void testSafetyEnvelope() {
    const gridex::SafetyEnvelope safety;

    auto discharge = safety.apply(gridex::SafetyInput{
        .requestedPowerKw = 200.0,
        .battery = goodBattery(),
        .site = {},
        .configuredLimit = {},
        .emsConnected = true,
    });
    assert(near(discharge.appliedPowerKw, 120.0));
    assert(discharge.clamped);

    auto charge = safety.apply(gridex::SafetyInput{
        .requestedPowerKw = -90.0,
        .battery = goodBattery(),
        .site = {.siteLoadKw = 470.0, .contractLimitKw = 500.0},
        .configuredLimit = {},
        .emsConnected = true,
    });
    assert(near(charge.appliedPowerKw, -30.0));
    assert(charge.clamped);

    auto stale = safety.apply(gridex::SafetyInput{
        .requestedPowerKw = 40.0,
        .battery = goodBattery(),
        .site = {},
        .configuredLimit = {},
        .emsConnected = false,
    });
    assert(near(stale.appliedPowerKw, 0.0));

    auto prohibitedBattery = goodBattery();
    prohibitedBattery.bmsSystemFlags = 0x0001U;
    auto prohibitedCharge = safety.apply(gridex::SafetyInput{
        .requestedPowerKw = -40.0,
        .battery = prohibitedBattery,
        .site = {},
        .configuredLimit = {},
        .emsConnected = true,
    });
    assert(near(prohibitedCharge.appliedPowerKw, 0.0));

    auto faultedBattery = goodBattery();
    faultedBattery.pcsFault = true;
    faultedBattery.controlReady = false;
    auto faulted = safety.apply(gridex::SafetyInput{
        .requestedPowerKw = 40.0,
        .battery = faultedBattery,
        .site = {},
        .configuredLimit = {},
        .emsConnected = true,
    });
    assert(near(faulted.appliedPowerKw, 0.0));

    auto configuredCap = safety.apply(gridex::SafetyInput{
        .requestedPowerKw = 100.0,
        .battery = goodBattery(),
        .site = {},
        .configuredLimit = {.maxChargeKw = 40.0, .maxDischargeKw = 55.0},
        .emsConnected = true,
    });
    assert(near(configuredCap.appliedPowerKw, 55.0));
}

void testSunStorageDriver() {
    FakeModbusClient modbus;
    using R = gridex::SunStoragePro261Driver::Registers;

    modbus.coils[R::PcsFault] = false;
    modbus.coils[R::PcsOffGrid] = false;
    modbus.coils[R::PcsWarning] = false;
    modbus.coils[R::PcsCommunicationFault] = false;
    modbus.coils[R::BmsAlarm] = false;
    modbus.coils[R::BmsFault] = false;
    modbus.coils[R::BmsCommunicationFault] = false;
    modbus.holding[R::PcsGridMode] = 0;
    modbus.holding[R::PcsWorkMode] = 1;
    modbus.holding[R::PcsPowerOn] = 1;
    modbus.holding[R::PcsSocUpperLimit] = 90;
    modbus.holding[R::PcsSocLowerLimit] = 10;
    modbus.input[R::PcsActivePower] = 250;
    modbus.input[R::PcsReactivePower] = 35;
    modbus.input[R::PcsDcPower] = 260;
    modbus.input[R::PcsFrequency] = 5000;
    modbus.input[R::PcsCommandedPower] = 250;
    modbus.input[R::PcsStatus] = 3;
    modbus.input[R::BmsCurrent] = static_cast<std::uint16_t>(
        static_cast<std::int16_t>(-123)
    );
    modbus.input[R::BmsSoc] = 725;
    modbus.input[R::BmsSoh] = 98;
    modbus.input[R::BmsVoltage] = 7600;
    modbus.input[R::BmsStatus] = 3;
    modbus.input[R::BmsAccumulatedChargeEnergy] = 0;
    modbus.input[R::BmsAccumulatedChargeEnergy + 1U] = 12345;
    modbus.input[R::BmsAccumulatedDischargeEnergy] = 1;
    modbus.input[R::BmsAccumulatedDischargeEnergy + 1U] = 33229;
    modbus.input[R::BmsSystemFlags] = 0;
    modbus.input[R::BmsMaxChargePower] = 1000;
    modbus.input[R::BmsMaxDischargePower] = 1200;
    modbus.input[R::BmsDailyChargeEnergy] = 125;
    modbus.input[R::BmsDailyDischargeEnergy] = 250;

    gridex::SunStoragePro261Driver locked(modbus);
    assert(!locked.writePowerSetpointKw(25.0));

    gridex::SunStoragePro261Driver unconfirmedEnergyOrder(
        modbus,
        {.addressingConfirmed = true,
         .signConfirmed = true,
         .scalingConfirmed = true}
    );
    const auto unconfirmedValue = unconfirmedEnergyOrder.poll();
    assert(unconfirmedValue.controlReady);
    assert(!unconfirmedValue.extendedTelemetryValid);

    gridex::SunStoragePro261Driver driver(
        modbus,
        {.addressingConfirmed = true,
         .signConfirmed = true,
         .scalingConfirmed = true,
         .int32WordOrderConfirmed = true,
         .int32HighWordFirst = true}
    );

    const auto value = driver.poll();
    assert(value.quality == gridex::Quality::Good);
    assert(near(value.socPct, 72.5));
    assert(near(value.currentA, -12.3));
    assert(near(value.maxChargeKw, 100.0));
    assert(near(value.dcPowerKw, 26.0));
    assert(near(value.commandedPowerKw, 25.0));
    assert(near(value.reactivePowerKvar, 3.5));
    assert(near(value.frequencyHz, 50.0));
    assert(near(value.accumulatedChargeKwh, 1234.5));
    assert(near(value.accumulatedDischargeKwh, 9876.5));
    assert(near(value.dailyChargeKwh, 12.5));
    assert(near(value.dailyDischargeKwh, 25.0));
    assert(value.pcsStatusCode == 3U);
    assert(value.extendedTelemetryValid);
    assert(value.controlReady);

    assert(driver.writePowerSetpointKw(-25.4));
    assert(
        static_cast<std::int16_t>(modbus.holding[R::PcsPowerCommand]) == -254
    );
    assert(driver.writeReactivePowerSetpointKvar(12.3));
    assert(
        static_cast<std::int16_t>(
            modbus.holding[R::PcsReactivePowerCommand]
        ) == 123
    );
    assert(driver.refreshHeartbeat(60));
    assert(modbus.holding[R::HeartbeatEnable] == 1);
    assert(modbus.holding[R::HeartbeatSeconds] == 60);
    assert(driver.writeSocLimitsPct(15.0, 85.0));
    assert(modbus.holding[R::PcsSocLowerLimit] == 15U);
    assert(modbus.holding[R::PcsSocUpperLimit] == 85U);
    assert(!driver.writeSocLimitsPct(90.0, 80.0));
    assert(driver.writePowerState(false));
    assert(modbus.holding[R::PcsPowerCommand] == 0U);
    assert(modbus.holding[R::PcsPowerOn] == 0U);
    assert(modbus.writeLog[modbus.writeLog.size() - 2U].first ==
           R::PcsPowerCommand);
    assert(modbus.writeLog.back().first == R::PcsPowerOn);
    modbus.holding[R::PcsPowerOn] = 1U;
    assert(driver.writePowerState(true));

    assert(!driver.writePowerSetpointKw(120.1));
    assert(!driver.writePowerSetpointKw(-100.1));

    modbus.input[R::BmsSystemFlags] = 0x0001U;
    const auto chargeProhibited = driver.poll();
    assert(chargeProhibited.controlReady);
    assert(!driver.writePowerSetpointKw(-10.0));
    assert(driver.writePowerSetpointKw(10.0));

    modbus.coils[R::PcsFault] = true;
    const auto faulted = driver.poll();
    assert(!faulted.controlReady);
    assert(!driver.writePowerSetpointKw(10.0));
}

}  // namespace

int main() {
    testSafetyEnvelope();
    testSunStorageDriver();
    std::cout << "All GrideX Edge tests passed.\n";
    return 0;
}
