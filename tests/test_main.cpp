#include "gridex/IModbusClient.hpp"
#include "gridex/SafetyEnvelope.hpp"
#include "gridex/SunStoragePro261Driver.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <map>
#include <optional>

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
        holding[address] = value;
        return writesSucceed;
    }

    std::map<std::uint16_t, std::uint16_t> input;
    std::map<std::uint16_t, std::uint16_t> holding;
    std::map<std::uint16_t, bool> coils;
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
        .alarmBits = 0,
        .quality = gridex::Quality::Good,
        .limitsValid = true,
    };
}

void testSafetyEnvelope() {
    const gridex::SafetyEnvelope safety;

    auto discharge = safety.apply(gridex::SafetyInput{
        .requestedPowerKw = 200.0,
        .battery = goodBattery(),
        .site = {},
        .emsConnected = true,
    });
    assert(near(discharge.appliedPowerKw, 120.0));
    assert(discharge.clamped);

    auto charge = safety.apply(gridex::SafetyInput{
        .requestedPowerKw = -90.0,
        .battery = goodBattery(),
        .site = {.siteLoadKw = 470.0, .contractLimitKw = 500.0},
        .emsConnected = true,
    });
    assert(near(charge.appliedPowerKw, -30.0));
    assert(charge.clamped);

    auto stale = safety.apply(gridex::SafetyInput{
        .requestedPowerKw = 40.0,
        .battery = goodBattery(),
        .site = {},
        .emsConnected = false,
    });
    assert(near(stale.appliedPowerKw, 0.0));
}

void testSunStorageDriver() {
    FakeModbusClient modbus;
    using R = gridex::SunStoragePro261Driver::Registers;

    modbus.input[R::PcsActivePower] = 250;
    modbus.input[R::BmsCurrent] = static_cast<std::uint16_t>(
        static_cast<std::int16_t>(-123)
    );
    modbus.input[R::BmsSoc] = 725;
    modbus.input[R::BmsSoh] = 98;
    modbus.input[R::BmsVoltage] = 7600;
    modbus.input[R::BmsStatus] = 3;
    modbus.input[R::BmsAlarmBits] = 0;
    modbus.input[R::BmsMaxChargePower] = 1000;
    modbus.input[R::BmsMaxDischargePower] = 1200;

    gridex::SunStoragePro261Driver locked(modbus);
    assert(!locked.writePowerSetpointKw(25.0));

    gridex::SunStoragePro261Driver driver(
        modbus,
        {.addressingConfirmed = true,
         .signConfirmed = true,
         .scalingConfirmed = true}
    );

    const auto value = driver.poll();
    assert(value.quality == gridex::Quality::Good);
    assert(near(value.socPct, 72.5));
    assert(near(value.currentA, -12.3));
    assert(near(value.maxChargeKw, 100.0));

    assert(driver.writePowerSetpointKw(-25.4));
    assert(
        static_cast<std::int16_t>(modbus.holding[R::PcsPowerCommand]) == -254
    );
    assert(driver.refreshHeartbeat(60));
    assert(modbus.holding[R::HeartbeatEnable] == 1);
    assert(modbus.holding[R::HeartbeatSeconds] == 60);
}

}  // namespace

int main() {
    testSafetyEnvelope();
    testSunStorageDriver();
    std::cout << "All GrideX Edge tests passed.\n";
    return 0;
}
