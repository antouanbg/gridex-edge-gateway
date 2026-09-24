#include "gridex/rockpie/MqttHealthPublisher.hpp"
#include "gridex/rockpie/CpuTemperature.hpp"

#include <cassert>
#include <string>
#include <sstream>
#include <limits>

std::size_t mqttPublisherSizeWithoutBuildFlag();

int main() {
    using gridex::rockpie::EdgeHealthMessage;
    using gridex::rockpie::MbusNodeTelemetry;
    using gridex::rockpie::MqttHealthPublisher;
    assert(sizeof(MqttHealthPublisher) == mqttPublisherSizeWithoutBuildFlag());

    const auto health = MqttHealthPublisher::healthPayload({
        .siteId = "test-site", .gatewayId = "edge-01", .state = "degraded",
        .pcsHeartbeatOk = false, .controlReady = false, .safeMode = true,
        .northboundReady = true, .nodeOnlineCount = 1, .nodeTotal = 2,
    });
    assert(health.find("\"schemaVersion\":1") != std::string::npos);
    assert(health.find("\"state\":\"degraded\"") != std::string::npos);
    assert(health.find("\"safeMode\":true") != std::string::npos);
    assert(health.find("\"cpuTemperatureC\":null") != std::string::npos);
    EdgeHealthMessage temperature;
    temperature.cpuTemperatureC = 48.125;
    assert(MqttHealthPublisher::healthPayload(temperature).find("\"cpuTemperatureC\":48.125") != std::string::npos);
    temperature.cpuTemperatureC = std::numeric_limits<double>::quiet_NaN();
    assert(MqttHealthPublisher::healthPayload(temperature).find("\"cpuTemperatureC\":null") != std::string::npos);
    for (const auto text : {"", "nan", "48000junk", "150001", "-40001", "48000 extra"}) {
        std::istringstream input(text);
        assert(!gridex::rockpie::parseCpuTemperature(input));
    }
    std::istringstream valid("48125\n");
    assert(gridex::rockpie::parseCpuTemperature(valid) == 48.125);
    std::istringstream zero("0\n");
    assert(gridex::rockpie::parseCpuTemperature(zero) == 0.0);
    assert(!gridex::rockpie::readCpuTemperature("/nonexistent-gridex-sensor"));

    const auto telemetry = MqttHealthPublisher::nodeTelemetryPayload(1, MbusNodeTelemetry{
        .address = 1, .nodeType = 3, .nodeState = 2, .driverId = 7,
        .quality = 0, .heartbeat = 42, .actualPowerKw = -12.5,
        .energyWh = 1234, .deviceState = 4, .alarmBits = 0, .online = true,
        .pollStatus = gridex::rockpie::NodePollStatus::Online,
        .consecutiveFailures = 2, .ethernetStatus = 2, .modbusTcpStatus = 1,
        .driverReady = 0, .deviceBusStatus = 1, .watchdogStatus = 1,
        .recoveryCount = 3, .lastError = 2,
    });
    assert(telemetry.find("\"slot\":1") != std::string::npos);
    assert(telemetry.find("\"actualPowerKw\":-12.5") != std::string::npos);
    assert(telemetry.find("\"online\":true") != std::string::npos);
    assert(telemetry.find("\"pollStatus\":1") != std::string::npos);
    assert(telemetry.find("\"recoveryCount\":3") != std::string::npos);
    assert(telemetry.find("\"lastSuccessfulContactAt\":null") != std::string::npos);
    MbusNodeTelemetry sample;
    sample.lastSuccessfulContact = std::chrono::system_clock::from_time_t(1000);
    const auto first = MqttHealthPublisher::nodeTelemetryPayload(1, sample);
    sample.online = false;
    const auto failed = MqttHealthPublisher::nodeTelemetryPayload(1, sample);
    const std::string contact = "\"lastSuccessfulContactAt\":\"1970-01-01T00:16:40Z\"";
    assert(first.find(contact) != std::string::npos);
    assert(failed.find(contact) != std::string::npos);
}
