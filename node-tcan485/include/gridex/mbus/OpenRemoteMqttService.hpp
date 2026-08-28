#pragma once

#ifdef ARDUINO

#include "gridex/mbus/IDeviceDriver.hpp"

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

namespace gridex::mbus {

struct OpenRemoteMqttConfig {
    bool enabled{false};
    String wifiSsid;
    String wifiPassword;
    String host;
    std::uint16_t port{8883};
    String realm{"master"};
    String serviceUser;
    String serviceSecret;
    String clientId;
    String assetId;
    String caCertificate;
};

class OpenRemoteMqttService {
public:
    explicit OpenRemoteMqttService(OpenRemoteMqttConfig config);

    bool begin();
    void loop();
    void publish(const DriverSample& sample, std::uint16_t heartbeat);
    [[nodiscard]] bool connected();

private:
    OpenRemoteMqttConfig config_;
    WiFiClientSecure tls_;
    PubSubClient mqtt_;
    unsigned long nextReconnectMs_{0};
    bool active_{false};

    bool connectMqtt();
    bool publishValue(const char* attribute, const String& value);
    [[nodiscard]] String topic(const char* attribute) const;
};

}  // namespace gridex::mbus

#endif
