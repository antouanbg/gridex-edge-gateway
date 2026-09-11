#pragma once

#include <cstddef>
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

// Publishes only non-secret health data. Its broker is reachable through the
// Site Router's VPN route; this class never establishes WireGuard itself.
class MqttHealthPublisher {
  public:
    MqttHealthPublisher(std::string brokerUrl, std::string topicPrefix);
    ~MqttHealthPublisher();
    MqttHealthPublisher(const MqttHealthPublisher&) = delete;
    MqttHealthPublisher& operator=(const MqttHealthPublisher&) = delete;
    bool publish(const EdgeHealthMessage& message) noexcept;
    bool enabled() const noexcept;
  private:
    std::string brokerUrl_;
    std::string topicPrefix_;
#ifdef GRIDEX_WITH_MOSQUITTO
    void* client_{nullptr};
#endif
};

}  // namespace gridex::rockpie
