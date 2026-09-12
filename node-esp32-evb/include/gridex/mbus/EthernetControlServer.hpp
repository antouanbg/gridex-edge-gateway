#pragma once

#ifdef ARDUINO

#include "gridex/mbus/MbusNode.hpp"

#include <Arduino.h>
#include <WiFi.h>

#include <cstdint>
#include <vector>

namespace gridex::mbus {

struct EthernetControlConfig {
    std::uint16_t port{1502};
    std::uint8_t unitId{1};
    IPAddress rockPiAddress{};
};

// Local OT-only Modbus TCP endpoint. It accepts writes exclusively from the
// configured ROCK Pi E address and only to the command register window.
class EthernetControlServer {
public:
    EthernetControlServer(MbusNode& node, EthernetControlConfig config);

    void begin();
    void restart();
    void loop();
    void setRockPiAddress(IPAddress address);
    [[nodiscard]] bool connected();
    [[nodiscard]] bool listening() const;
    [[nodiscard]] std::uint32_t rejectedClients() const;

private:
    MbusNode& node_;
    EthernetControlConfig config_;
    WiFiServer server_;
    WiFiClient client_;
    std::vector<std::uint8_t> rx_;
    std::uint32_t rejectedClients_{0};
    unsigned long lastByteMs_{0};

    void acceptClient();
    void processFrames();
    [[nodiscard]] bool sourceAllowed(const WiFiClient& client) const;
    [[nodiscard]] bool writeAllowed(
        std::uint8_t function,
        std::uint16_t start,
        std::uint16_t count
    ) const;
    [[nodiscard]] std::vector<std::uint8_t> handle(
        const std::uint8_t* adu,
        std::size_t size
    );
};

}  // namespace gridex::mbus

#endif
