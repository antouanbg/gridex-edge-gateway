#ifdef ARDUINO

#include "gridex/mbus/BoardPins.hpp"
#include "gridex/mbus/IDeviceDriver.hpp"
#include "gridex/mbus/MbusNode.hpp"
#include "gridex/mbus/OpenRemoteMqttService.hpp"

#include <Arduino.h>
#include <Preferences.h>

#include <memory>
#include <vector>

namespace {

Preferences preferences;
Preferences cloudPreferences;
std::unique_ptr<gridex::mbus::MbusNode> node;
std::unique_ptr<gridex::mbus::IDeviceDriver> driver;
std::unique_ptr<gridex::mbus::OpenRemoteMqttService> cloud;
std::vector<std::uint8_t> rxFrame;
unsigned long lastRxUs = 0;
unsigned long lastPollMs = 0;
unsigned long lastCloudPublishMs = 0;
gridex::mbus::DriverSample lastSample;

gridex::mbus::NodeConfig loadConfig() {
    preferences.begin("gridex-mbus", false);
    gridex::mbus::NodeConfig config;
    config.address = preferences.getUChar("address", 0);
    config.type = static_cast<gridex::mbus::NodeType>(
        preferences.getUShort("node_type", 0)
    );
    config.driverId = preferences.getUShort("driver_id", 0);
    config.uid = ESP.getEfuseMac();
    return config;
}

gridex::mbus::OpenRemoteMqttConfig loadCloudConfig() {
    cloudPreferences.begin("gridex-cloud", true);
    gridex::mbus::OpenRemoteMqttConfig config;
    config.enabled = cloudPreferences.getBool("enabled", false);
    config.wifiSsid = cloudPreferences.getString("wifi_ssid", "");
    config.wifiPassword = cloudPreferences.getString("wifi_pass", "");
    config.host = cloudPreferences.getString("mqtt_host", "");
    config.port = cloudPreferences.getUShort("mqtt_port", 8883);
    config.realm = cloudPreferences.getString("realm", "master");
    config.serviceUser = cloudPreferences.getString("mqtt_user", "");
    config.serviceSecret = cloudPreferences.getString("mqtt_secret", "");
    config.clientId = cloudPreferences.getString("client_id", "");
    config.assetId = cloudPreferences.getString("asset_id", "");
    config.caCertificate = cloudPreferences.getString("ca_cert", "");
    cloudPreferences.end();
    return config;
}

void persistConfig() {
    preferences.putUChar("address", node->address());
    preferences.putUShort(
        "node_type",
        static_cast<std::uint16_t>(node->type())
    );
    preferences.putUShort("driver_id", node->driverId());
}

void publishDriverSample(const gridex::mbus::DriverSample& sample) {
    node->setRegister(
        gridex::mbus::reg::ActualPowerKwX10,
        static_cast<std::uint16_t>(sample.actualPowerKwX10)
    );
    node->setRegister(
        gridex::mbus::reg::EnergyWhHigh,
        static_cast<std::uint16_t>(sample.energyWh >> 16U)
    );
    node->setRegister(
        gridex::mbus::reg::EnergyWhLow,
        static_cast<std::uint16_t>(sample.energyWh)
    );
    node->setRegister(gridex::mbus::reg::DeviceState, sample.state);
    node->setRegister(gridex::mbus::reg::AlarmBits, sample.alarms);
    node->setRegister(gridex::mbus::reg::Quality, sample.quality);
}

void processCompleteFrame() {
    if (rxFrame.empty()) {
        return;
    }
    const auto response = node->processFrame(rxFrame.data(), rxFrame.size());
    rxFrame.clear();
    if (!response.empty()) {
        Serial1.write(response.data(), response.size());
        Serial1.flush();
    }
    if (node->takeConfigurationChanged()) {
        persistConfig();
        driver = std::make_unique<gridex::mbus::UnconfiguredDriver>(
            node->type(),
            node->driverId()
        );
        driver->begin();
    }
}

}  // namespace

void setup() {
    pinMode(gridex::mbus::board::Rs485Enable, OUTPUT);
    pinMode(gridex::mbus::board::BoosterEnable, OUTPUT);
    pinMode(gridex::mbus::board::Rs485Callback, INPUT);
    pinMode(gridex::mbus::board::StatusLed, OUTPUT);
    digitalWrite(gridex::mbus::board::Rs485Enable, HIGH);
    digitalWrite(gridex::mbus::board::BoosterEnable, HIGH);

    Serial.begin(115200);
    Serial1.begin(
        gridex::mbus::board::Baud,
        SERIAL_8N1,
        gridex::mbus::board::Rs485Rx,
        gridex::mbus::board::Rs485Tx
    );

    node = std::make_unique<gridex::mbus::MbusNode>(loadConfig());
    driver = std::make_unique<gridex::mbus::UnconfiguredDriver>(
        node->type(),
        node->driverId()
    );
    driver->begin();
    cloud = std::make_unique<gridex::mbus::OpenRemoteMqttService>(
        loadCloudConfig()
    );
    cloud->begin();
    rxFrame.reserve(256);
    digitalWrite(gridex::mbus::board::StatusLed, HIGH);
}

void loop() {
    cloud->loop();

    while (Serial1.available() > 0) {
        const int value = Serial1.read();
        if (value >= 0 && rxFrame.size() < 256U) {
            rxFrame.push_back(static_cast<std::uint8_t>(value));
            lastRxUs = micros();
        }
    }
    if (!rxFrame.empty() &&
        static_cast<unsigned long>(micros() - lastRxUs) >=
            gridex::mbus::board::FrameSilenceUs) {
        processCompleteFrame();
    }

    if (millis() - lastPollMs >= 500U) {
        lastPollMs = millis();
        lastSample = driver->poll();
        publishDriverSample(lastSample);
        node->setRegister(
            gridex::mbus::reg::UptimeLow,
            static_cast<std::uint16_t>(millis() / 1000U)
        );
        node->setRegister(
            gridex::mbus::reg::Heartbeat,
            static_cast<std::uint16_t>(
                node->registerValue(gridex::mbus::reg::Heartbeat) + 1U
            )
        );
        node->setRegister(
            gridex::mbus::reg::CloudConnected,
            cloud->connected() ? 1U : 0U
        );
    }
    if (millis() - lastCloudPublishMs >= 2000U) {
        lastCloudPublishMs = millis();
        cloud->publish(
            lastSample,
            node->registerValue(gridex::mbus::reg::Heartbeat)
        );
    }
    delay(1);
}

#endif
