#include "gridex/EdgeController.hpp"
#include "gridex/rockpie/NodeTcpPollingService.hpp"
#include "gridex/rockpie/NorthboundModbusTcpServer.hpp"
#include "gridex/rockpie/PosixModbusTcpClient.hpp"
#include "gridex/rockpie/MqttHealthPublisher.hpp"
#include "gridex/rockpie/TelemetryJournal.hpp"
#include "gridex/rockpie/CpuTemperature.hpp"
#include "gridex/rockpie/SystemTelemetry.hpp"

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

std::size_t envSize(const char* name, std::size_t fallback,
                    std::size_t minimum, std::size_t maximum) {
    try {
        const auto value = std::stoull(envString(name, std::to_string(fallback).c_str()));
        return std::clamp(static_cast<std::size_t>(value), minimum, maximum);
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
    // Initialize MQTT before starting any worker threads and keep it alive
    // until those workers have stopped (reverse destruction order).  This is
    // required by libmosquitto's global pthread state on small ARM systems.
    gridex::rockpie::MqttHealthPublisher healthPublisher({
        .brokerUrl = envString("GRIDEX_MQTT_BROKER_URL", ""),
        .topicPrefix = envString("GRIDEX_MQTT_TOPIC_PREFIX", "gridex/v1"),
        .clientId = envString("GRIDEX_MQTT_CLIENT_ID", ""),
        .username = envString("GRIDEX_MQTT_USERNAME", ""),
        .passwordFile = envString("GRIDEX_MQTT_PASSWORD_FILE", ""),
        .caFile = envString("GRIDEX_MQTT_CA_FILE", ""),
        .clientCertificateFile = envString("GRIDEX_MQTT_CLIENT_CERT_FILE", ""),
        .clientKeyFile = envString("GRIDEX_MQTT_CLIENT_KEY_FILE", ""),
    });
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
    const auto healthInterval = std::chrono::seconds(
        std::clamp(envInt("GRIDEX_HEALTH_PUBLISH_SECONDS", 10), 2, 300));
    const bool cpuTemperatureEnabled = envBool("GRIDEX_CPU_TEMPERATURE_ENABLED", false);
    const auto cpuTemperaturePath = envString("GRIDEX_CPU_TEMPERATURE_FILE", "/sys/class/thermal/thermal_zone0/temp");
    const auto telemetryInterval = std::chrono::seconds(
        std::clamp(envInt("GRIDEX_NODE_TELEMETRY_PUBLISH_SECONDS", 2), 1, 300));
    const bool systemTelemetryEnabled = envBool("GRIDEX_SYSTEM_TELEMETRY_ENABLED", false);
    const auto systemTelemetryInterval = std::chrono::seconds(std::clamp(envInt("GRIDEX_SYSTEM_TELEMETRY_PUBLISH_SECONDS", 30), 5, 3600));
    const auto systemDataDirectory = envString("GRIDEX_SYSTEM_DATA_DIRECTORY", "/var/lib/gridex");
    const auto bootId = envString("GRIDEX_BOOT_ID", "unknown");
    std::uint64_t systemTelemetrySequence = 0;
    gridex::rockpie::TelemetryJournal telemetryJournal({
        .path = envString("GRIDEX_TELEMETRY_JOURNAL_PATH",
                          "/var/lib/gridex/telemetry-journal.ndjson"),
        .maxBytes = envSize("GRIDEX_TELEMETRY_JOURNAL_MAX_BYTES",
                            4U * 1024U * 1024U,
                            64U * 1024U,
                            64U * 1024U * 1024U),
        .enabled = envBool("GRIDEX_TELEMETRY_JOURNAL_ENABLED", true),
    });
    const auto journalInterval = std::chrono::seconds(
        std::clamp(envInt("GRIDEX_TELEMETRY_JOURNAL_SECONDS", 5), 1, 300));
    std::vector<gridex::rockpie::NodePollStatus> journalStatuses;
    auto nextHealthPublish = std::chrono::steady_clock::now();
    auto nextTelemetryPublish = std::chrono::steady_clock::now();
    auto nextJournalSnapshot = std::chrono::steady_clock::now();
    auto nextSystemTelemetryPublish = std::chrono::steady_clock::now();

    std::cout << "GrideX ROCK Pi E service started; writes_enabled="
              << (driver.writesEnabled() ? "true" : "false") << '\n';
    if (telemetryJournal.enabled()) {
        std::cout << "GrideX local telemetry journal enabled\n";
    }

    while (running) {
        healthPublisher.pump();
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
        if (journalStatuses.size() != nodeSamples.size()) {
            journalStatuses.assign(nodeSamples.size(), gridex::rockpie::NodePollStatus::Unknown);
        }
        for (std::size_t slot = 0; slot < nodeSamples.size(); ++slot) {
            northboundBank.publishNode(slot, nodeSamples[slot]);
            if (nodeSamples[slot].pollStatus != journalStatuses[slot]) {
                if (!telemetryJournal.appendTransition(
                        slot + 1U, nodeSamples[slot], journalStatuses[slot])) {
                    std::cerr << "GrideX telemetry journal transition write failed\n";
                }
                journalStatuses[slot] = nodeSamples[slot].pollStatus;
            }
        }
        std::cout << "{\"state\":\"" << stateName(snapshot.state)
                  << "\",\"soc_pct\":" << snapshot.battery.socPct
                  << ",\"actual_kw\":" << snapshot.battery.actualPowerKw
                  << ",\"applied_kw\":" << snapshot.command.appliedPowerKw
                  << ",\"heartbeat_ok\":"
                  << (snapshot.heartbeatOk ? "true" : "false")
                  << ",\"reason\":\"" << snapshot.command.reason << "\"}\n";
        if (now >= nextHealthPublish) {
            const auto onlineNodes = static_cast<std::size_t>(std::count_if(
                nodeSamples.begin(), nodeSamples.end(),
                [](const auto& sample) { return sample.online; }));
            const bool safeMode = snapshot.state == gridex::EdgeState::SafeMode;
            const bool controlReady = snapshot.state == gridex::EdgeState::Ready;
            (void)healthPublisher.publishHealth({
                .siteId = envString("GRIDEX_SITE_ID", ""),
                .gatewayId = envString("GRIDEX_GATEWAY_ID", ""),
                .state = safeMode ? "safe_mode" : (controlReady ? "ready" : "degraded"),
                .pcsHeartbeatOk = snapshot.heartbeatOk,
                .controlReady = controlReady,
                .safeMode = safeMode,
                .northboundReady = true,
                .nodeOnlineCount = onlineNodes,
                .nodeTotal = nodeSamples.size(),
                .cpuTemperatureC = cpuTemperatureEnabled
                    ? gridex::rockpie::readCpuTemperature(cpuTemperaturePath) : std::nullopt,
            });
            nextHealthPublish = now + healthInterval;
        }
        if (now >= nextTelemetryPublish) {
            const auto siteId = envString("GRIDEX_SITE_ID", "");
            const auto gatewayId = envString("GRIDEX_GATEWAY_ID", "");
            for (std::size_t slot = 0; slot < nodeSamples.size(); ++slot) {
                (void)healthPublisher.publishNodeTelemetry(
                    siteId, gatewayId, slot + 1U, nodeSamples[slot]);
            }
            nextTelemetryPublish = now + telemetryInterval;
        }
        // Do not enter the system telemetry/filesystem path while MQTT is
        // offline.  This keeps the edge loop stable during broker/TLS
        // outages; the next interval retries automatically after reconnect.
        if (systemTelemetryEnabled && healthPublisher.connected() && now >= nextSystemTelemetryPublish) {
            const auto samples = gridex::rockpie::readSystemTelemetry(systemDataDirectory, telemetryJournal.path(), cpuTemperaturePath, cpuTemperatureEnabled);
            const auto sequence = ++systemTelemetrySequence;
            const bool published = healthPublisher.publishSystemTelemetry(
                envString("GRIDEX_SITE_ID", ""), envString("GRIDEX_GATEWAY_ID", ""),
                bootId, sequence, samples);
            if (!published) {
                std::cerr << "{\"system_telemetry_publish\":false,\"sequence\":"
                          << sequence << ",\"samples\":" << samples.size() << "}\n";
            } else {
                std::cout << "{\"system_telemetry_publish\":true,\"sequence\":"
                          << sequence << ",\"samples\":" << samples.size() << "}\n";
            }
            nextSystemTelemetryPublish = now + systemTelemetryInterval;
        }
        if (now >= nextJournalSnapshot) {
            for (std::size_t slot = 0; slot < nodeSamples.size(); ++slot) {
                if (!telemetryJournal.appendSnapshot(slot + 1U, nodeSamples[slot])) {
                    std::cerr << "GrideX telemetry journal snapshot write failed\n";
                }
            }
            nextJournalSnapshot = now + journalInterval;
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
