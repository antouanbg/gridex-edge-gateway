#include "gridex/rockpie/MqttHealthPublisher.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string_view>

#ifdef GRIDEX_WITH_MOSQUITTO
#include <mosquitto.h>
#endif

namespace gridex::rockpie {
namespace {
#ifdef GRIDEX_WITH_MOSQUITTO
struct BrokerEndpoint { std::string host; int port{1883}; };
std::optional<BrokerEndpoint> parseBrokerUrl(const std::string& url) {
    constexpr std::string_view prefix{"mqtt://"};
    if (!url.starts_with(prefix)) return std::nullopt;
    const auto endpoint = url.substr(prefix.size());
    const auto colon = endpoint.rfind(':');
    if (colon == std::string::npos || colon == 0) return std::nullopt;
    try { return BrokerEndpoint{endpoint.substr(0, colon), std::stoi(endpoint.substr(colon + 1))}; }
    catch (...) { return std::nullopt; }
}
std::string timestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t value = std::chrono::system_clock::to_time_t(now);
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &value);
#else
    gmtime_r(&value, &utc);
#endif
    std::ostringstream output;
    output << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return output.str();
}
const char* jsonBoolean(bool value) { return value ? "true" : "false"; }
#endif
}  // namespace

MqttHealthPublisher::MqttHealthPublisher(std::string brokerUrl, std::string topicPrefix)
    : brokerUrl_(std::move(brokerUrl)), topicPrefix_(std::move(topicPrefix)) {
#ifdef GRIDEX_WITH_MOSQUITTO
    if (brokerUrl_.empty() || topicPrefix_.empty()) return;
    mosquitto_lib_init();
    client_ = mosquitto_new(nullptr, true, nullptr);
    const auto endpoint = parseBrokerUrl(brokerUrl_);
    if (!client_ || !endpoint || endpoint->port < 1 || endpoint->port > 65535 ||
        mosquitto_connect_async(static_cast<mosquitto*>(client_), endpoint->host.c_str(), endpoint->port, 30) != MOSQ_ERR_SUCCESS ||
        mosquitto_loop_start(static_cast<mosquitto*>(client_)) != MOSQ_ERR_SUCCESS) {
        if (client_) mosquitto_destroy(static_cast<mosquitto*>(client_));
        client_ = nullptr;
    }
#endif
}

MqttHealthPublisher::~MqttHealthPublisher() {
#ifdef GRIDEX_WITH_MOSQUITTO
    if (client_) { mosquitto_loop_stop(static_cast<mosquitto*>(client_), true); mosquitto_destroy(static_cast<mosquitto*>(client_)); }
    mosquitto_lib_cleanup();
#endif
}

bool MqttHealthPublisher::enabled() const noexcept {
#ifdef GRIDEX_WITH_MOSQUITTO
    return client_ != nullptr;
#else
    return false;
#endif
}

bool MqttHealthPublisher::publish(const EdgeHealthMessage& message) noexcept {
#ifdef GRIDEX_WITH_MOSQUITTO
    if (!client_ || message.siteId.empty() || message.gatewayId.empty()) return false;
    const std::string topic = topicPrefix_ + "/sites/" + message.siteId + "/edge/" + message.gatewayId + "/health";
    std::ostringstream payloadStream;
    payloadStream << "{\"schemaVersion\":1,\"gatewayId\":\"" << message.gatewayId
            << "\",\"observedAt\":\"" << timestamp() << "\",\"state\":\"" << message.state
            << "\",\"pcsHeartbeatOk\":" << jsonBoolean(message.pcsHeartbeatOk)
            << ",\"controlReady\":" << jsonBoolean(message.controlReady)
            << ",\"safeMode\":" << jsonBoolean(message.safeMode)
            << ",\"northboundReady\":" << jsonBoolean(message.northboundReady)
            << ",\"nodeOnlineCount\":" << message.nodeOnlineCount
            << ",\"nodeTotal\":" << message.nodeTotal << "}";
    const auto payload = payloadStream.str();
    // The private listener is reachable only through the Site Router VPN.
    return mosquitto_publish(static_cast<mosquitto*>(client_), nullptr, topic.c_str(),
        static_cast<int>(payload.size()), payload.c_str(), 1, false) == MOSQ_ERR_SUCCESS;
#else
    (void)message;
    return false;
#endif
}

}  // namespace gridex::rockpie
