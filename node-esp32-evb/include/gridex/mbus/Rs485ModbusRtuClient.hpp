#pragma once

#ifdef ARDUINO

#include <Arduino.h>

#include <cstdint>
#include <optional>

namespace gridex::mbus {

// Minimal, fail-closed Modbus RTU master for one direct UART/RS-485 device.
// It is intentionally separate from the northbound Modbus TCP server.
class Rs485ModbusRtuClient {
public:
    Rs485ModbusRtuClient(
        HardwareSerial& serial,
        std::uint8_t unitId,
        int directionPin,
        std::uint32_t timeoutMs = 300
    );

    void begin(std::uint32_t baud, std::uint32_t serialConfig, int rxPin, int txPin);
    [[nodiscard]] std::optional<std::uint16_t> readHolding(std::uint16_t address);
    [[nodiscard]] bool writeHolding(std::uint16_t address, std::uint16_t value);

private:
    HardwareSerial& serial_;
    std::uint8_t unitId_;
    int directionPin_;
    std::uint32_t timeoutMs_;

    [[nodiscard]] static std::uint16_t crc16(const std::uint8_t* bytes, std::size_t size);
    void drainInput();
};

}  // namespace gridex::mbus

#endif
