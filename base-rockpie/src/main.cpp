#include "gridex/EdgeController.hpp"
#include "gridex/rockpie/MbusPollingService.hpp"
#include "gridex/rockpie/MbusRtuClient.hpp"
#include "gridex/rockpie/NorthboundModbusTcpServer.hpp"
#include "gridex/rockpie/PosixModbusTcpClient.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace {

std::atomic_bool running{true};

void stopService(int) {
    running = false;
}

std::string envString(const char* name, const char* fallback) {
    const char* value = std::getenv(name);
    return value && *value ? value : fallback;
}

int envInt(const char* name, int fallback) {
    try {
        return std::stoi(envString(name, std::to_string(fallback).c_str()));
    } catch (...) {
        return fallback;
    }
}

bool envBool(const char* name, bool fallback = false) {
    const auto value = envString(name, fallback ? "1" : "0");
    return value == "1" || value == "true" || value == "yes";
}

std::optional<double> envOptionalDouble(const char* name) {
    const auto value = envString(name, "");
    if (value.empty()) {
        return std::nullopt;
    }
    try {
        return std::stod(value);
    } catch (...) {
        return std::nullopt;
    }
}

std::vector<std::uint8_t> envAddressList(const char* name) {
    std::vector<std::uint8_t> addresses;
    std::stringstream input(envString(name, ""));
    std::string item;
    while (std::getline(input, item, ',')) {
        try {
            const int value = std::stoi(item);
            if (value >= 1 && value <= 247) {
                addresses.push_back(static_cast<std::uint8_t>(value));
            }
        } catch (...) {
        }
    }
    return addresses;
}

const char* stateName(gridex::EdgeState state) {
    switch (state) {
        case gridex::EdgeState::Boot: return "boot";
        case gridex::EdgeState::WaitingForComms: return "waiting_for_comms";
        case gridex::EdgeState::WaitingForBmsLimits: return "waiting_for_bms_limits";
        case gridex::EdgeState::Ready: return "ready";
        case gridex::EdgeState::SafeMode: return "safe_mode";
        case gridex::EdgeState::Fault: return "fault";
        case gridex::EdgeState::CommissioningLocked: return "commissioning_locked";
        case gridex::EdgeState::WaitingForPcsReady: return "waiting_for_pcs_ready";
    }
    return "unknown";
}

}  // namespace

int main() {
    std::signal(SIGINT, stopService);
    std::signal(SIGTERM, stopService);

    gridex::rockpie::PosixModbusTcpClient transport({
        .host = envString("GRIDEX_PCS_HOST", "192.168.50.10"),
        .port = static_cast<std::uint16_t>(envInt("GRIDEX_PCS_PORT", 3200)),
        .unitId = static_cast<std::uint8_t>(envInt("GRIDEX_PCS_UNIT_ID", 1)),
        .timeout = std::chrono::milliseconds(envInt("GRIDEX_PCS_TIMEOUT_MS", 800)),
    });

    gridex::SunStoragePro261Driver driver(
        transport,
        {
            .addressingConfirmed = envBool("GRIDEX_APPROVE_ADDRESSING"),
            .signConfirmed = envBool("GRIDEX_APPROVE_POWER_SIGN"),
            .scalingConfirmed = envBool("GRIDEX_APPROVE_SCALING"),
            .int32WordOrderConfirmed =
                envBool("GRIDEX_APPROVE_INT32_WORD_ORDER"),
            .int32HighWordFirst =
                envBool("GRIDEX_INT32_HIGH_WORD_FIRST", true),
        }
    );
    gridex::ControllerConfig controllerConfig;
    controllerConfig.configuredLimit.maxChargeKw =
        envOptionalDouble("GRIDEX_MAX_CHARGE_KW");
    controllerConfig.configuredLimit.maxDischargeKw =
        envOptionalDouble("GRIDEX_MAX_DISCHARGE_KW");
    gridex::EdgeController controller(
        driver,
        gridex::SafetyEnvelope{},
        controllerConfig
    );
    gridex::rockpie::NorthboundRegisterBank northboundBank;
    gridex::rockpie::NorthboundModbusTcpServer northboundServer(
        northboundBank,
        {
            .bindAddress = envString("GRIDEX_NORTHBOUND_BIND", "0.0.0.0"),
            .port = static_cast<std::uint16_t>(
                envInt("GRIDEX_NORTHBOUND_PORT", 1502)
            ),
            .unitId = static_cast<std::uint8_t>(
                envInt("GRIDEX_NORTHBOUND_UNIT_ID", 1)
            ),
        }
    );
    if (!northboundServer.start()) {
        std::cerr << "Cannot start northbound Modbus TCP server\n";
        return 1;
    }

    gridex::rockpie::MbusRtuClient mbus({
        .device = envString("GRIDEX_MBUS_DEVICE", "/dev/ttyS1"),
        .baud = static_cast<std::uint32_t>(
            envInt("GRIDEX_MBUS_BAUD", 115200)
        ),
        .timeout = std::chrono::milliseconds(
            envInt("GRIDEX_MBUS_TIMEOUT_MS", 80)
        ),
    });
    if (envBool("GRIDEX_MBUS_SCAN_ON_START")) {
        for (const auto& node : mbus.scan()) {
            std::cout << "MBUS node address=" << static_cast<int>(node.address)
                      << " type=" << node.nodeType
                      << " driver_id=" << node.driverId
                      << " uid=" << node.uid << '\n';
        }
    }
    gridex::rockpie::MbusPollingService mbusPolling(
        mbus,
        {
            .addresses = envAddressList("GRIDEX_MBUS_NODE_ADDRESSES"),
            .interval = std::chrono::milliseconds(
                envInt("GRIDEX_MBUS_POLL_MS", 500)
            ),
        }
    );
    mbusPolling.start();

    std::cout << "GrideX ROCK Pi E service started; writes_enabled="
              << (driver.writesEnabled() ? "true" : "false") << '\n';

    while (running) {
        const auto now = std::chrono::steady_clock::now();
        if (const auto command = northboundBank.takeCommand()) {
            controller.receiveCommand(command->requestedPowerKw, {}, now);
        }
        const auto snapshot = controller.tick(now);
        if (const auto operation = northboundBank.takeOperatorCommand()) {
            std::uint16_t result = 1U;  // success
            const bool maskValid = operation->actionMask != 0U &&
                (operation->actionMask & ~gridex::northbound::action::All) == 0U;
            if (!operation->authorized) {
                result = 2U;  // invalid apply key
            } else if (!maskValid ||
                       ((operation->actionMask &
                         gridex::northbound::action::RunState) != 0U &&
                        operation->requestedRunState > 1U)) {
                result = 3U;  // invalid request
            } else {
                bool accepted = true;
                if ((operation->actionMask &
                     gridex::northbound::action::RunState) != 0U) {
                    accepted = driver.writePowerState(
                        operation->requestedRunState == 1U
                    ) && accepted;
                }
                if ((operation->actionMask &
                     gridex::northbound::action::SocLimits) != 0U) {
                    accepted = driver.writeSocLimitsPct(
                        operation->requestedSocLowerPct,
                        operation->requestedSocUpperPct
                    ) && accepted;
                }
                if ((operation->actionMask &
                     gridex::northbound::action::ReactivePower) != 0U) {
                    accepted = driver.writeReactivePowerSetpointKvar(
                        operation->requestedReactivePowerKvar
                    ) && accepted;
                }
                if (!accepted) result = 4U;  // safety/driver rejection
            }
            northboundBank.publishOperatorResult(operation->sequence, result);
        }
        northboundBank.publish(snapshot, controllerConfig.configuredLimit);
        const auto nodeSamples = mbusPolling.samples();
        for (std::size_t slot = 0; slot < nodeSamples.size(); ++slot) {
            northboundBank.publishNode(slot, nodeSamples[slot]);
        }
        std::cout << "{\"state\":\"" << stateName(snapshot.state)
                  << "\",\"soc_pct\":" << snapshot.battery.socPct
                  << ",\"actual_kw\":" << snapshot.battery.actualPowerKw
                  << ",\"applied_kw\":" << snapshot.command.appliedPowerKw
                  << ",\"heartbeat_ok\":"
                  << (snapshot.heartbeatOk ? "true" : "false")
                  << ",\"reason\":\"" << snapshot.command.reason << "\"}\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(
            envInt("GRIDEX_TICK_MS", 1000)
        ));
    }

    mbusPolling.stop();
    northboundServer.stop();
    transport.disconnect();
    std::cout << "GrideX ROCK Pi E service stopped safely\n";
    return 0;
}
