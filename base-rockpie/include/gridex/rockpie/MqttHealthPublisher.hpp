#pragma once

#include "gridex/rockpie/MbusNodeTelemetry.hpp"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <optional>
#include <vector>

namespace gridex::rockpie {

// The connection is intentionally outbound-only.  MQTT carries telemetry and
// health through the Site Router VPN; it is not a device-control transport.
struct MqttHealthPublisherConfig {
    std::string brokerUrl;
    std::string topicPrefix{"gridex/v1"};
    std::string clientId;
    std::string username;
    std::string passwordFile;
    std::string caFile;
    std::string clientCertificateFile;
    std::string clientKeyFile;
};

struct EdgeHealthMessage {
    std::string siteId;
    std::string gatewayId;
    std::string state;
    bool pcsHeartbeatOk{false};
    bool controlReady{false};
    bool safeMode{false};
    bool northboundReady{false};
    std::size_t nodeOnlineCount{0};
    std::size_t nodeTotal{0};
    std::optional<double> cpuTemperatureC;
};

struct SystemTelemetrySample {
    std::string sensorId;
    std::string unit;
    double value{0.0};
};

class MqttHealthPublisher {
  public:
    explicit MqttHealthPublisher(MqttHealthPublisherConfig config);
    ~MqttHealthPublisher();
    MqttHealthPublisher(const MqttHealthPublisher&) = delete;
    MqttHealthPublisher& operator=(const MqttHealthPublisher&) = delete;

    [[nodiscard]] bool configured() const noexcept;
    [[nodiscard]] bool connected() const noexcept;
    bool publishHealth(const EdgeHealthMessage& message) noexcept;
    bool publishNodeTelemetry(const std::string& siteId,
                              const std::string& gatewayId,
                              std::size_t slot,
                              const MbusNodeTelemetry& sample) noexcept;
    bool publishSystemTelemetry(const std::string& siteId, const std::string& gatewayId,
                                const std::string& bootId, std::uint64_t sequence,
                                const std::vector<SystemTelemetrySample>& samples) noexcept;

    // Kept public for deterministic tests, independent of a local broker.
    [[nodiscard]] static std::string healthPayload(const EdgeHealthMessage& message);
    [[nodiscard]] static std::string nodeTelemetryPayload(
        std::size_t slot, const MbusNodeTelemetry& sample);
    [[nodiscard]] static std::string systemTelemetryPayload(
        const std::string& gatewayId, const std::string& bootId, std::uint64_t sequence,
        const std::vector<SystemTelemetrySample>& samples);

  private:
    MqttHealthPublisherConfig config_;
    // Keep the public header's class layout independent of the library's
    // private build flag. The service and this library are separate C++ TUs.
    void* client_{nullptr};
    std::atomic_bool connected_{false};
    bool libraryInitialized_{false};
    bool loopStarted_{false};
    static void onConnect(void* client, void* context, int result) noexcept;
    static void onDisconnect(void* client, void* context, int result) noexcept;
};

}  // namespace gridex::rockpie
