#ifdef ARDUINO

#include "gridex/mbus/EthernetControlServer.hpp"
#include "gridex/mbus/IDeviceDriver.hpp"
#include "gridex/mbus/MbusNode.hpp"
#include "gridex/mbus/MqttTelemetryService.hpp"
#include "gridex/node/BoardPins.hpp"

#include <Arduino.h>
#include <ETH.h>
#include <Preferences.h>
#if defined(GRIDEX_DEVICE_BUS_CAN)
#include <driver/twai.h>
#endif

#include <memory>

namespace {

Preferences preferences;
Preferences cloudPreferences;
std::unique_ptr<gridex::mbus::MbusNode> node;
std::unique_ptr<gridex::mbus::IDeviceDriver> driver;
std::unique_ptr<gridex::mbus::MqttTelemetryService> cloud;
std::unique_ptr<gridex::mbus::EthernetControlServer> control;
unsigned long lastPollMs = 0;
unsigned long lastCloudPublishMs = 0;
unsigned long lastCommandMs = 0;
std::uint16_t lastCommandSequence = 0;
bool commandArmed = false;
gridex::mbus::DriverSample lastSample;

gridex::mbus::NodeConfig loadConfig() {
    preferences.begin("gridex-mbus", false);
    gridex::mbus::NodeConfig config;
    config.address = 1;
    config.type = static_cast<gridex::mbus::NodeType>(
        preferences.getUShort("node_type", 0)
    );
    config.driverId = preferences.getUShort("driver_id", 0);
    config.uid = ESP.getEfuseMac();
    preferences.end();
    return config;
}

gridex::mbus::MqttTelemetryConfig loadCloudConfig() {
    cloudPreferences.begin("gridex-cloud", true);
    gridex::mbus::MqttTelemetryConfig config;
    config.enabled = cloudPreferences.getBool("enabled", false);
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

gridex::mbus::EthernetControlConfig loadControlConfig() {
    cloudPreferences.begin("gridex-control", true);
    gridex::mbus::EthernetControlConfig config;
    config.port = cloudPreferences.getUShort("port", 1502);
    config.unitId = 1;
    const auto rockPi = cloudPreferences.getString("rockpi_ip", "");
    config.rockPiAddress.fromString(rockPi);
    cloudPreferences.end();
    return config;
}

bool initializeDeviceBus() {
#if defined(GRIDEX_DEVICE_BUS_CAN)
    twai_general_config_t general = TWAI_GENERAL_CONFIG_DEFAULT(
        static_cast<gpio_num_t>(gridex::node::board::CanTx),
        static_cast<gpio_num_t>(gridex::node::board::CanRx),
        TWAI_MODE_NORMAL
    );
    twai_timing_config_t timing = TWAI_TIMING_CONFIG_250KBITS();
    twai_filter_config_t filter = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    return twai_driver_install(&general, &timing, &filter) == ESP_OK &&
           twai_start() == ESP_OK;
#elif defined(GRIDEX_DEVICE_BUS_RS485)
    const int directionPin = GRIDEX_RS485_DIRECTION_GPIO;
    pinMode(directionPin, OUTPUT);
    digitalWrite(directionPin, LOW);
    Serial1.begin(
        9600,
        SERIAL_8N1,
        gridex::node::board::Rs485Rx,
        gridex::node::board::Rs485Tx
    );
    return true;
#else
    return false;
#endif
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

void applyCommand(unsigned long nowMs) {
    const auto sequence = node->registerValue(
        gridex::mbus::reg::CommandSequence
    );
    const auto ttlSeconds = node->registerValue(
        gridex::mbus::reg::CommandTtlSeconds
    );
    if (sequence != lastCommandSequence) {
        lastCommandSequence = sequence;
        lastCommandMs = nowMs;
        const bool enabled =
            node->registerValue(gridex::mbus::reg::CommandEnable) == 1U;
        const bool ttlValid = ttlSeconds >= 1U && ttlSeconds <= 30U;
        const auto requested = static_cast<std::int16_t>(
            node->registerValue(gridex::mbus::reg::RequestedPowerKwX10)
        );
        const bool accepted = enabled && ttlValid &&
                              driver->applyPowerCommand(requested);
        node->setRegister(
            gridex::mbus::reg::AppliedPowerKwX10,
            accepted ? static_cast<std::uint16_t>(requested) : 0U
        );
        node->setRegister(
            gridex::mbus::reg::LastCommandResult,
            accepted ? 1U : (ttlValid ? 2U : 4U)
        );
        commandArmed = accepted;
    }
    if (commandArmed &&
        nowMs - lastCommandMs > static_cast<unsigned long>(ttlSeconds) * 1000U) {
        driver->applyPowerCommand(0);
        node->setRegister(gridex::mbus::reg::AppliedPowerKwX10, 0U);
        node->setRegister(gridex::mbus::reg::LastCommandResult, 3U);
        commandArmed = false;
    }
}

}  // namespace

void setup() {
    Serial.begin(115200);
    // OLIMEX documents a short delay before ETH.begin() to avoid PHY init
    // failures after reset.
    delay(2000);
    ETH.begin();
    if (!initializeDeviceBus()) {
        Serial.println("GrideX: device bus initialization failed");
    }

    node = std::make_unique<gridex::mbus::MbusNode>(loadConfig());
    driver = std::make_unique<gridex::mbus::UnconfiguredDriver>(
        node->type(),
        node->driverId()
    );
    driver->begin();
    cloud = std::make_unique<gridex::mbus::MqttTelemetryService>(
        loadCloudConfig()
    );
    cloud->begin();
    control = std::make_unique<gridex::mbus::EthernetControlServer>(
        *node,
        loadControlConfig()
    );
    control->begin();
}

void loop() {
    cloud->loop();
    control->loop();
    applyCommand(millis());

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
