#include "gridex/rockpie/MqttHealthPublisher.hpp"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <utility>

#ifdef GRIDEX_WITH_MOSQUITTO
#include <mosquitto.h>
#endif

namespace gridex::rockpie {
namespace {

struct BrokerEndpoint {
    std::string host;
    int port{8883};
};

std::string jsonString(std::string_view value) {
    std::ostringstream output;
    output << '"';
    for (const auto character : value) {
        switch (character) {
            case '"': output << "\\\""; break;
            case '\\': output << "\\\\"; break;
            case '\n': output << "\\n"; break;
            case '\r': output << "\\r"; break;
            case '\t': output << "\\t"; break;
            default:
                if (static_cast<unsigned char>(character) < 0x20U) {
                    output << "\\u00" << std::hex << std::setw(2)
                           << std::setfill('0')
                           << static_cast<int>(static_cast<unsigned char>(character))
                           << std::dec << std::setfill(' ');
                } else {
                    output << character;
                }
        }
    }
    output << '"';
    return output.str();
}

std::string timestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto seconds = std::chrono::system_clock::to_time_t(now);
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &seconds);
#else
    gmtime_r(&seconds, &utc);
#endif
    std::ostringstream output;
    output << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return output.str();
}

const char* jsonBoolean(bool value) { return value ? "true" : "false"; }

#ifdef GRIDEX_WITH_MOSQUITTO
std::string topicPart(std::string_view value) {
    if (value.empty() || value.find_first_of("/+#") != std::string_view::npos) {
        return {};
    }
    return std::string{value};
}

bool parseBrokerUrl(std::string_view url, BrokerEndpoint& output) {
    constexpr std::string_view prefix{"mqtts://"};
    if (!url.starts_with(prefix)) return false;
    const auto endpoint = url.substr(prefix.size());
    const auto separator = endpoint.rfind(':');
    if (separator == std::string_view::npos || separator == 0U ||
        separator + 1U >= endpoint.size()) return false;
    try {
        const auto port = std::stoi(std::string{endpoint.substr(separator + 1U)});
        if (port < 1 || port > 65535) return false;
        output = BrokerEndpoint{.host = std::string{endpoint.substr(0, separator)}, .port = port};
        return true;
    } catch (...) {
        return false;
    }
}

std::string readSecretFile(const std::string& path) {
    if (path.empty()) return {};
    std::ifstream input(path);
    std::string value;
    std::getline(input, value);
    return value;
}
#endif

}  // namespace

MqttHealthPublisher::MqttHealthPublisher(MqttHealthPublisherConfig config)
    : config_(std::move(config)) {
#ifdef GRIDEX_WITH_MOSQUITTO
    BrokerEndpoint endpoint;
    if (!configured() || !parseBrokerUrl(config_.brokerUrl, endpoint) ||
        config_.caFile.empty()) return;
    if (config_.clientCertificateFile.empty() != config_.clientKeyFile.empty()) return;
    if (mosquitto_lib_init() != MOSQ_ERR_SUCCESS) return;
    libraryInitialized_ = true;
    client_ = mosquitto_new(config_.clientId.empty() ? nullptr : config_.clientId.c_str(), true, this);
    if (!client_) return;
    const auto password = readSecretFile(config_.passwordFile);
    const bool clientCertificatePair = !config_.clientCertificateFile.empty() || !config_.clientKeyFile.empty();
    const bool tlsConfigured = mosquitto_tls_set(
        static_cast<mosquitto*>(client_), config_.caFile.c_str(), nullptr,
        clientCertificatePair ? config_.clientCertificateFile.c_str() : nullptr,
        clientCertificatePair ? config_.clientKeyFile.c_str() : nullptr, nullptr) == MOSQ_ERR_SUCCESS;
    const bool authConfigured = (config_.username.empty() && password.empty()) ||
        mosquitto_username_pw_set(static_cast<mosquitto*>(client_),
            config_.username.empty() ? nullptr : config_.username.c_str(),
            password.empty() ? nullptr : password.c_str()) == MOSQ_ERR_SUCCESS;
    const bool secure = mosquitto_tls_insecure_set(static_cast<mosquitto*>(client_), false) == MOSQ_ERR_SUCCESS;
    mosquitto_connect_callback_set(static_cast<mosquitto*>(client_),
        [](mosquitto* client, void* context, int result) {
            MqttHealthPublisher::onConnect(client, context, result);
        });
    mosquitto_disconnect_callback_set(static_cast<mosquitto*>(client_),
        [](mosquitto* client, void* context, int result) {
            MqttHealthPublisher::onDisconnect(client, context, result);
        });
    if (!tlsConfigured || !authConfigured || !secure ||
        mosquitto_connect_async(static_cast<mosquitto*>(client_), endpoint.host.c_str(), endpoint.port, 30) != MOSQ_ERR_SUCCESS ||
        mosquitto_loop_start(static_cast<mosquitto*>(client_)) != MOSQ_ERR_SUCCESS) {
        mosquitto_destroy(static_cast<mosquitto*>(client_));
        client_ = nullptr;
    }
#endif
}

MqttHealthPublisher::~MqttHealthPublisher() {
#ifdef GRIDEX_WITH_MOSQUITTO
    if (client_) {
        mosquitto_disconnect(static_cast<mosquitto*>(client_));
        mosquitto_loop_stop(static_cast<mosquitto*>(client_), true);
        mosquitto_destroy(static_cast<mosquitto*>(client_));
    }
    if (libraryInitialized_) mosquitto_lib_cleanup();
#endif
}

bool MqttHealthPublisher::configured() const noexcept {
    return !config_.brokerUrl.empty() && !config_.topicPrefix.empty() &&
        !config_.clientId.empty();
}

bool MqttHealthPublisher::connected() const noexcept {
#ifdef GRIDEX_WITH_MOSQUITTO
    return client_ != nullptr && connected_;
#else
    return false;
#endif
}

std::string MqttHealthPublisher::healthPayload(const EdgeHealthMessage& message) {
    std::ostringstream output;
    output << "{\"schemaVersion\":1,\"observedAt\":" << jsonString(timestamp())
           << ",\"gatewayId\":" << jsonString(message.gatewayId)
           << ",\"state\":" << jsonString(message.state)
           << ",\"pcsHeartbeatOk\":" << jsonBoolean(message.pcsHeartbeatOk)
           << ",\"controlReady\":" << jsonBoolean(message.controlReady)
           << ",\"safeMode\":" << jsonBoolean(message.safeMode)
           << ",\"northboundReady\":" << jsonBoolean(message.northboundReady)
           << ",\"nodeOnlineCount\":" << message.nodeOnlineCount
           << ",\"nodeTotal\":" << message.nodeTotal << "}";
    return output.str();
}

std::string MqttHealthPublisher::nodeTelemetryPayload(
    std::size_t slot, const MbusNodeTelemetry& sample) {
    std::ostringstream output;
    output << "{\"schemaVersion\":1,\"observedAt\":" << jsonString(timestamp())
           << ",\"slot\":" << slot
           << ",\"nodeAddress\":" << static_cast<unsigned int>(sample.address)
           << ",\"nodeType\":" << sample.nodeType
           << ",\"nodeState\":" << sample.nodeState
           << ",\"driverId\":" << sample.driverId
           << ",\"online\":" << jsonBoolean(sample.online)
           << ",\"quality\":" << sample.quality
           << ",\"actualPowerKw\":" << sample.actualPowerKw
           << ",\"energyWh\":" << sample.energyWh
           << ",\"deviceState\":" << sample.deviceState
           << ",\"alarmBits\":" << sample.alarmBits
           << ",\"heartbeat\":" << sample.heartbeat << "}";
    return output.str();
}

bool MqttHealthPublisher::publishHealth(const EdgeHealthMessage& message) noexcept {
#ifdef GRIDEX_WITH_MOSQUITTO
    const auto siteId = topicPart(message.siteId);
    const auto gatewayId = topicPart(message.gatewayId);
    if (!connected() || siteId.empty() || gatewayId.empty()) return false;
    const auto topic = config_.topicPrefix + "/sites/" + siteId + "/edge/" + gatewayId + "/health";
    const auto payload = healthPayload(message);
    return mosquitto_publish(static_cast<mosquitto*>(client_), nullptr, topic.c_str(),
        static_cast<int>(payload.size()), payload.c_str(), 1, true) == MOSQ_ERR_SUCCESS;
#else
    (void)message;
    return false;
#endif
}

bool MqttHealthPublisher::publishNodeTelemetry(
    const std::string& siteId, const std::string& gatewayId, std::size_t slot,
    const MbusNodeTelemetry& sample) noexcept {
#ifdef GRIDEX_WITH_MOSQUITTO
    const auto site = topicPart(siteId);
    const auto gateway = topicPart(gatewayId);
    if (!connected() || site.empty() || gateway.empty() || slot == 0U) return false;
    const auto topic = config_.topicPrefix + "/sites/" + site + "/edge/" + gateway +
        "/nodes/" + std::to_string(slot) + "/telemetry";
    const auto payload = nodeTelemetryPayload(slot, sample);
    return mosquitto_publish(static_cast<mosquitto*>(client_), nullptr, topic.c_str(),
        static_cast<int>(payload.size()), payload.c_str(), 1, false) == MOSQ_ERR_SUCCESS;
#else
    (void)siteId;
    (void)gatewayId;
    (void)slot;
    (void)sample;
    return false;
#endif
}

#ifdef GRIDEX_WITH_MOSQUITTO
void MqttHealthPublisher::onConnect(void*, void* context, int result) noexcept {
    auto* publisher = static_cast<MqttHealthPublisher*>(context);
    if (publisher) publisher->connected_ = result == MOSQ_ERR_SUCCESS;
}

void MqttHealthPublisher::onDisconnect(void*, void* context, int) noexcept {
    auto* publisher = static_cast<MqttHealthPublisher*>(context);
    if (publisher) publisher->connected_ = false;
}
#endif

}  // namespace gridex::rockpie
