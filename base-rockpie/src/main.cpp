#include "gridex/EdgeController.hpp"
#include "gridex/rockpie/MbusRtuClient.hpp"
#include "gridex/rockpie/PosixModbusTcpClient.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

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

const char* stateName(gridex::EdgeState state) {
    switch (state) {
        case gridex::EdgeState::Boot: return "boot";
        case gridex::EdgeState::WaitingForComms: return "waiting_for_comms";
        case gridex::EdgeState::WaitingForBmsLimits: return "waiting_for_bms_limits";
        case gridex::EdgeState::Ready: return "ready";
        case gridex::EdgeState::SafeMode: return "safe_mode";
        case gridex::EdgeState::Fault: return "fault";
        case gridex::EdgeState::CommissioningLocked: return "commissioning_locked";
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
        }
    );
    gridex::EdgeController controller(driver, gridex::SafetyEnvelope{});

    if (envBool("GRIDEX_MBUS_SCAN_ON_START")) {
        gridex::rockpie::MbusRtuClient mbus({
            .device = envString("GRIDEX_MBUS_DEVICE", "/dev/ttyS1"),
            .baud = static_cast<std::uint32_t>(
                envInt("GRIDEX_MBUS_BAUD", 115200)
            ),
            .timeout = std::chrono::milliseconds(
                envInt("GRIDEX_MBUS_TIMEOUT_MS", 80)
            ),
        });
        for (const auto& node : mbus.scan()) {
            std::cout << "MBUS node address=" << static_cast<int>(node.address)
                      << " type=" << node.nodeType
                      << " uid=" << node.uid << '\n';
        }
    }

    std::cout << "GrideX ROCK Pi E service started; writes_enabled="
              << (driver.writesEnabled() ? "true" : "false") << '\n';

    while (running) {
        const auto snapshot = controller.tick(std::chrono::steady_clock::now());
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

    transport.disconnect();
    std::cout << "GrideX ROCK Pi E service stopped safely\n";
    return 0;
}
