#include "gridex/EdgeController.hpp"
#include "gridex/rockpie/NodeTcpPollingService.hpp"
#include "gridex/rockpie/NorthboundModbusTcpServer.hpp"
#include "gridex/rockpie/PosixModbusTcpClient.hpp"
#include "gridex/rockpie/MqttHealthPublisher.hpp"

#include <atomic>
#include <algorithm>
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

std::vector<gridex::rockpie::NodeTcpEndpoint> envNodeEndpoints(
    const char* name
) {
    std::vector<gridex::rockpie::NodeTcpEndpoint> endpoints;
    std::stringstream input(envString(name, ""));
    std::string item;
    while (std::getline(input, item, ',')) {
        const auto separator = item.rfind(':');
        const auto host = separator == std::string::npos
            ? item
            : item.substr(0, separator);
        if (host.empty()) continue;
        int port = 1502;
        if (separator != std::string::npos) {
            try {
                port = std::stoi(item.substr(separator + 1));
            } catch (...) {
                continue;
            }
        }
        if (port < 1 || port > 65535) continue;
        endpoints.push_back({
            .host = host,
            .port = static_cast<std::uint16_t>(port),
            .unitId = 1,
        });
    }
    return endpoints;
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

    gridex::rockpie::NodeTcpPollingService nodePolling({
        .endpoints = envNodeEndpoints("GRIDEX_NODE_ENDPOINTS"),
        .interval = std::chrono::milliseconds(
            envInt("GRIDEX_NODE_POLL_MS", 500)
        ),
        .timeout = std::chrono::milliseconds(
            envInt("GRIDEX_NODE_TIMEOUT_MS", 400)
        ),
    });
    nodePolling.start();
    gridex::rockpie::MqttHealthPublisher healthPublisher(
        envString("GRIDEX_MQTT_BROKER_URL", ""),
        envString("GRIDEX_MQTT_TOPIC_PREFIX", "gridex/v1")
    );
    const auto healthInterval = std::chrono::seconds(envInt("GRIDEX_HEALTH_PUBLISH_SECONDS", 10));
    auto nextHealthPublish = std::chrono::steady_clock::now();

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
        if (const auto nodeCommand = northboundBank.takeNodeCommand()) {
            const bool accepted = nodeCommand->authorized &&
                nodePolling.applyPowerCommand(
                    nodeCommand->targetSlot,
                    nodeCommand->requestedPowerKw,
                    nodeCommand->enabled,
                    nodeCommand->sequence,
                    nodeCommand->ttlSeconds
                );
            std::cout << "{\"node_command_slot\":"
                      << nodeCommand->targetSlot
                      << ",\"accepted\":"
                      << (accepted ? "true" : "false") << "}\n";
        }
        northboundBank.publish(snapshot, controllerConfig.configuredLimit);
        const auto nodeSamples = nodePolling.samples();
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
        if (std::chrono::steady_clock::now() >= nextHealthPublish) {
            const auto onlineNodes = static_cast<std::size_t>(std::count_if(
                nodeSamples.begin(), nodeSamples.end(), [](const auto& node) { return node.online; }
            ));
            const bool safeMode = snapshot.state == gridex::EdgeState::SafeMode;
            const bool controlReady = snapshot.state == gridex::EdgeState::Ready;
            healthPublisher.publish({
                .siteId = envString("GRIDEX_SITE_ID", ""), .gatewayId = envString("GRIDEX_GATEWAY_ID", ""),
                .state = safeMode ? "safe_mode" : (controlReady ? "ready" : "degraded"),
                .pcsHeartbeatOk = snapshot.heartbeatOk, .controlReady = controlReady, .safeMode = safeMode,
                .northboundReady = true, .nodeOnlineCount = onlineNodes, .nodeTotal = nodeSamples.size(),
            });
            nextHealthPublish = std::chrono::steady_clock::now() + healthInterval;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(
            envInt("GRIDEX_TICK_MS", 1000)
        ));
    }

    nodePolling.stop();
    northboundServer.stop();
    transport.disconnect();
    std::cout << "GrideX ROCK Pi E service stopped safely\n";
    return 0;
}
