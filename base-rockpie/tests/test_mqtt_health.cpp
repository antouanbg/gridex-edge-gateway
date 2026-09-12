#include "gridex/rockpie/MqttHealthPublisher.hpp"

#include <cassert>
#include <string>

int main() {
    using gridex::rockpie::EdgeHealthMessage;
    using gridex::rockpie::MbusNodeTelemetry;
    using gridex::rockpie::MqttHealthPublisher;

    const auto health = MqttHealthPublisher::healthPayload({
        .siteId = "test-site", .gatewayId = "edge-01", .state = "degraded",
        .pcsHeartbeatOk = false, .controlReady = false, .safeMode = true,
        .northboundReady = true, .nodeOnlineCount = 1, .nodeTotal = 2,
    });
    assert(health.find("\"schemaVersion\":1") != std::string::npos);
    assert(health.find("\"state\":\"degraded\"") != std::string::npos);
    assert(health.find("\"safeMode\":true") != std::string::npos);

    const auto telemetry = MqttHealthPublisher::nodeTelemetryPayload(1, MbusNodeTelemetry{
        .address = 1, .nodeType = 3, .nodeState = 2, .driverId = 7,
        .quality = 0, .heartbeat = 42, .actualPowerKw = -12.5,
        .energyWh = 1234, .deviceState = 4, .alarmBits = 0, .online = true,
    });
    assert(telemetry.find("\"slot\":1") != std::string::npos);
    assert(telemetry.find("\"actualPowerKw\":-12.5") != std::string::npos);
    assert(telemetry.find("\"online\":true") != std::string::npos);
}
