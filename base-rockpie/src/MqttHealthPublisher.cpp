#include "gridex/rockpie/MqttHealthPublisher.hpp"

#include <chrono>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string_view>
#include <cstdlib>

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
std::optional<double> jsonNumber(std::string_view payload, std::string_view key) {
    const auto marker = std::string{"\""} + std::string{key} + "\"";
    const auto keyAt = payload.find(marker);
    if (keyAt == std::string_view::npos) return std::nullopt;
    const auto colon = payload.find(':', keyAt + marker.size());
    if (colon == std::string_view::npos) return std::nullopt;
    const auto start = payload.find_first_not_of(" \t\r\n", colon + 1U);
    if (start == std::string_view::npos) return std::nullopt;
    const std::string value{payload.substr(start)};
    char* end{};
    const double parsed = std::strtod(value.c_str(), &end);
    return end == value.c_str() ? std::nullopt : std::optional<double>{parsed};
}
std::optional<bool> jsonBooleanValue(std::string_view payload, std::string_view key) {
    const auto marker = std::string{"\""} + std::string{key} + "\"";
    const auto keyAt = payload.find(marker);
    if (keyAt == std::string_view::npos) return std::nullopt;
    const auto colon = payload.find(':', keyAt + marker.size());
    if (colon == std::string_view::npos) return std::nullopt;
    const auto start = payload.find_first_not_of(" \t\r\n", colon + 1U);
    if (start == std::string_view::npos) return std::nullopt;
    if (payload.substr(start, 4U) == "true") return true;
    if (payload.substr(start, 5U) == "false") return false;
    return std::nullopt;
}
#endif
}  // namespace

MqttHealthPublisher::MqttHealthPublisher(std::string brokerUrl, std::string topicPrefix)
    : brokerUrl_(std::move(brokerUrl)), topicPrefix_(std::move(topicPrefix)) {
#ifdef GRIDEX_WITH_MOSQUITTO
    if (brokerUrl_.empty() || topicPrefix_.empty()) return;
    mosquitto_lib_init();
    client_ = mosquitto_new(nullptr, true, this);
    const auto endpoint = parseBrokerUrl(brokerUrl_);
    if (!client_ || !endpoint || endpoint->port < 1 || endpoint->port > 65535 ||
        mosquitto_connect_async(static_cast<mosquitto*>(client_), endpoint->host.c_str(), endpoint->port, 30) != MOSQ_ERR_SUCCESS ||
        mosquitto_loop_start(static_cast<mosquitto*>(client_)) != MOSQ_ERR_SUCCESS) {
        if (client_) mosquitto_destroy(static_cast<mosquitto*>(client_));
        client_ = nullptr;
    }
    if (client_) {
        mosquitto_connect_callback_set(static_cast<mosquitto*>(client_),
            [](mosquitto* client, void* context, int result) {
                MqttHealthPublisher::onConnect(client, context, result);
            });
        mosquitto_message_callback_set(static_cast<mosquitto*>(client_),
            [](mosquitto* client, void* context, const mosquitto_message* message) {
                MqttHealthPublisher::onMessage(client, context, message);
            });
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

#ifdef GRIDEX_WITH_MOSQUITTO
bool MqttHealthPublisher::subscribeNodeCommands(
    const std::string& siteId,
    const std::string& gatewayId
) noexcept {
    if (!client_ || siteId.empty() || gatewayId.empty()) return false;
    std::scoped_lock lock(mutex_);
    if (subscribedSiteId_ == siteId && subscribedGatewayId_ == gatewayId) return true;
    const std::string topic = topicPrefix_ + "/sites/" + siteId + "/edge/" +
                              gatewayId + "/nodes/+/command";
    if (mosquitto_subscribe(static_cast<mosquitto*>(client_), nullptr, topic.c_str(), 1) != MOSQ_ERR_SUCCESS) {
        return false;
    }
    subscribedSiteId_ = siteId;
    subscribedGatewayId_ = gatewayId;
    return true;
}
#endif

bool MqttHealthPublisher::publishNodeTelemetry(
    const std::string& siteId,
    const std::string& gatewayId,
    std::size_t slot,
    const MbusNodeTelemetry& sample
) noexcept {
#ifdef GRIDEX_WITH_MOSQUITTO
    if (!client_ || siteId.empty() || gatewayId.empty() || slot == 0U) return false;
    (void)subscribeNodeCommands(siteId, gatewayId);
    const std::string topic = topicPrefix_ + "/sites/" + siteId + "/edge/" +
        gatewayId + "/nodes/" + std::to_string(slot) + "/telemetry";
    std::ostringstream payload;
    payload << "{\"schemaVersion\":1,\"observedAt\":\"" << timestamp()
            << "\",\"slot\":" << slot << ",\"driverId\":" << sample.driverId
            << ",\"nodeType\":" << sample.nodeType << ",\"online\":" << jsonBoolean(sample.online)
            << ",\"quality\":" << sample.quality << ",\"actualPowerKw\":" << sample.actualPowerKw
            << ",\"energyWh\":" << sample.energyWh << ",\"deviceState\":" << sample.deviceState
            << ",\"alarmBits\":" << sample.alarmBits << ",\"heartbeat\":" << sample.heartbeat << "}";
    const auto text = payload.str();
    return mosquitto_publish(static_cast<mosquitto*>(client_), nullptr, topic.c_str(),
        static_cast<int>(text.size()), text.c_str(), 1, false) == MOSQ_ERR_SUCCESS;
#else
    (void)siteId; (void)gatewayId; (void)slot; (void)sample;
    return false;
#endif
}

std::optional<NodeMqttCommand> MqttHealthPublisher::takeNodeCommand() {
    std::scoped_lock lock(mutex_);
    if (pendingCommands_.empty()) return std::nullopt;
    auto command = pendingCommands_.front();
    pendingCommands_.pop_front();
    return command;
}

#ifdef GRIDEX_WITH_MOSQUITTO
void MqttHealthPublisher::onConnect(void*, void* context, int result) noexcept {
    auto* bridge = static_cast<MqttHealthPublisher*>(context);
    if (!bridge || result != 0) return;
    std::scoped_lock lock(bridge->mutex_);
    // The next telemetry cycle resubscribes after a broker reconnect.
    bridge->subscribedSiteId_.clear();
    bridge->subscribedGatewayId_.clear();
}

void MqttHealthPublisher::onMessage(void*, void* context, const void* opaqueMessage) noexcept {
    auto* bridge = static_cast<MqttHealthPublisher*>(context);
    const auto* message = static_cast<const mosquitto_message*>(opaqueMessage);
    if (!bridge || !message || !message->topic || !message->payload || message->payloadlen <= 0 || message->payloadlen > 1024) return;
    const std::string_view topic{message->topic};
    constexpr std::string_view suffix{"/command"};
    const auto nodeMarker = topic.rfind("/nodes/");
    if (!topic.ends_with(suffix) || nodeMarker == std::string_view::npos) return;
    const auto slotText = topic.substr(nodeMarker + 7U, topic.size() - nodeMarker - 7U - suffix.size());
    const std::string slotValue{slotText};
    char* slotEnd{};
    const auto slot = std::strtoul(slotValue.c_str(), &slotEnd, 10);
    const std::string_view payload{static_cast<const char*>(message->payload), static_cast<std::size_t>(message->payloadlen)};
    const auto sequence = jsonNumber(payload, "sequence");
    const auto requestedPowerKw = jsonNumber(payload, "requestedPowerKw");
    const auto enabled = jsonBooleanValue(payload, "enabled");
    const auto ttlSeconds = jsonNumber(payload, "ttlSeconds");
    if (!slotEnd || *slotEnd != '\0' || slot == 0U || !sequence || !requestedPowerKw || !enabled || !ttlSeconds ||
        *sequence < 1.0 || *sequence > 65535.0 || std::floor(*sequence) != *sequence ||
        *ttlSeconds < 1.0 || *ttlSeconds > 30.0 || std::floor(*ttlSeconds) != *ttlSeconds ||
        *requestedPowerKw < -3276.8 || *requestedPowerKw > 3276.7) return;
    std::scoped_lock lock(bridge->mutex_);
    if (bridge->pendingCommands_.size() >= 16U) bridge->pendingCommands_.pop_front();
    bridge->pendingCommands_.push_back(NodeMqttCommand{.slot = static_cast<std::size_t>(slot),
        .sequence = static_cast<std::uint16_t>(*sequence), .requestedPowerKw = *requestedPowerKw,
        .enabled = *enabled, .ttlSeconds = static_cast<std::uint16_t>(*ttlSeconds)});
}
#endif

}  // namespace gridex::rockpie
