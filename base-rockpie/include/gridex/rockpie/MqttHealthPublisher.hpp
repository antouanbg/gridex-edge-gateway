#pragma once

#include "gridex/rockpie/MbusNodeTelemetry.hpp"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>
#include <optional>
#include <string>

namespace gridex::rockpie {

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
};

struct NodeMqttCommand {
    std::size_t slot{};
    std::uint16_t sequence{};
    double requestedPowerKw{};
    bool enabled{false};
    std::uint16_t ttlSeconds{};
};

// ROCK Pi is the only site MQTT bridge. ESP nodes use OT Modbus TCP only;
// telemetry flows node -> ROCK Pi -> MQTT and commands return through ROCK Pi.
class MqttHealthPublisher {
  public:
    MqttHealthPublisher(std::string brokerUrl, std::string topicPrefix);
    ~MqttHealthPublisher();
    MqttHealthPublisher(const MqttHealthPublisher&) = delete;
    MqttHealthPublisher& operator=(const MqttHealthPublisher&) = delete;
    bool publish(const EdgeHealthMessage& message) noexcept;
    bool publishNodeTelemetry(const std::string& siteId, const std::string& gatewayId,
                              std::size_t slot, const MbusNodeTelemetry& sample) noexcept;
    [[nodiscard]] std::optional<NodeMqttCommand> takeNodeCommand();
    bool enabled() const noexcept;
  private:
    std::string brokerUrl_;
    std::string topicPrefix_;
    std::string subscribedSiteId_;
    std::string subscribedGatewayId_;
    std::deque<NodeMqttCommand> pendingCommands_;
    mutable std::mutex mutex_;
#ifdef GRIDEX_WITH_MOSQUITTO
    void* client_{nullptr};
    bool subscribeNodeCommands(const std::string& siteId, const std::string& gatewayId) noexcept;
    static void onConnect(void* client, void* context, int result) noexcept;
    static void onMessage(void* client, void* context, const void* message) noexcept;
#endif
};

}  // namespace gridex::rockpie
