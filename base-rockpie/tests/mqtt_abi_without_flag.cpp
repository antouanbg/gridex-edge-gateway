// This TU simulates a consumer compiled without the optional MQTT flag.
#ifdef GRIDEX_WITH_MOSQUITTO
#undef GRIDEX_WITH_MOSQUITTO
#endif
#include "gridex/rockpie/MqttHealthPublisher.hpp"

std::size_t mqttPublisherSizeWithoutBuildFlag() {
    return sizeof(gridex::rockpie::MqttHealthPublisher);
}
