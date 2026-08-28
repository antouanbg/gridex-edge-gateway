#ifdef ARDUINO

#include "gridex/mbus/OpenRemoteMqttService.hpp"

#include <WiFi.h>
#include <utility>

namespace gridex::mbus {

OpenRemoteMqttService::OpenRemoteMqttService(OpenRemoteMqttConfig config)
    : config_(std::move(config)), mqtt_(tls_) {}

bool OpenRemoteMqttService::begin() {
    if (!config_.enabled || config_.wifiSsid.isEmpty() ||
        config_.host.isEmpty() || config_.serviceUser.isEmpty() ||
        config_.serviceSecret.isEmpty() || config_.clientId.isEmpty() ||
        config_.assetId.isEmpty() || config_.caCertificate.isEmpty()) {
        return false;
    }
    tls_.setCACert(config_.caCertificate.c_str());
    mqtt_.setServer(config_.host.c_str(), config_.port);
    mqtt_.setBufferSize(512U);
    mqtt_.setKeepAlive(30U);
    mqtt_.setSocketTimeout(5U);
    WiFi.mode(WIFI_STA);
    WiFi.begin(config_.wifiSsid.c_str(), config_.wifiPassword.c_str());
    active_ = true;
    return true;
}

String OpenRemoteMqttService::topic(const char* attribute) const {
    return config_.realm + "/" + config_.clientId +
           "/writeattributevalue/" + attribute + "/" + config_.assetId;
}

bool OpenRemoteMqttService::connectMqtt() {
    if (WiFi.status() != WL_CONNECTED) return false;
    String username = config_.serviceUser;
    if (username.indexOf(':') < 0) {
        username = config_.realm + ":" + username;
    }
    const auto willTopic = topic("online");
    const bool ok = mqtt_.connect(
        config_.clientId.c_str(),
        username.c_str(),
        config_.serviceSecret.c_str(),
        willTopic.c_str(),
        1,
        true,
        "false"
    );
    if (ok) publishValue("online", "true");
    return ok;
}

void OpenRemoteMqttService::loop() {
    if (!active_) return;
    if (WiFi.status() != WL_CONNECTED) {
        if (millis() >= nextReconnectMs_) {
            WiFi.disconnect();
            WiFi.begin(config_.wifiSsid.c_str(), config_.wifiPassword.c_str());
            nextReconnectMs_ = millis() + 5000U;
        }
        return;
    }
    if (!mqtt_.connected()) {
        if (millis() >= nextReconnectMs_) {
            connectMqtt();
            nextReconnectMs_ = millis() + 5000U;
        }
        return;
    }
    mqtt_.loop();
}

bool OpenRemoteMqttService::publishValue(
    const char* attribute,
    const String& value
) {
    const auto target = topic(attribute);
    return mqtt_.publish(target.c_str(), value.c_str(), false);
}

void OpenRemoteMqttService::publish(
    const DriverSample& sample,
    std::uint16_t heartbeat
) {
    if (!active_ || !mqtt_.connected()) return;
    publishValue("actualPowerKw", String(sample.actualPowerKwX10 / 10.0, 1));
    publishValue("energyWh", String(sample.energyWh));
    publishValue("deviceState", String(sample.state));
    publishValue("alarmBits", String(sample.alarms));
    publishValue("quality", String(sample.quality));
    publishValue("heartbeat", String(heartbeat));
}

bool OpenRemoteMqttService::connected() {
    return active_ && mqtt_.connected();
}

}  // namespace gridex::mbus

#endif
