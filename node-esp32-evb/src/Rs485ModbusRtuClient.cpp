#ifdef ARDUINO

#include "gridex/mbus/Rs485ModbusRtuClient.hpp"

#include <array>

namespace gridex::mbus {

Rs485ModbusRtuClient::Rs485ModbusRtuClient(
    HardwareSerial& serial,
    std::uint8_t unitId,
    int directionPin,
    std::uint32_t timeoutMs
) : serial_(serial), unitId_(unitId), directionPin_(directionPin), timeoutMs_(timeoutMs) {}

void Rs485ModbusRtuClient::begin(
    std::uint32_t baud,
    std::uint32_t serialConfig,
    int rxPin,
    int txPin
) {
    pinMode(directionPin_, OUTPUT);
    digitalWrite(directionPin_, LOW);
    serial_.begin(baud, serialConfig, rxPin, txPin);
}

std::uint16_t Rs485ModbusRtuClient::crc16(const std::uint8_t* bytes, std::size_t size) {
    std::uint16_t crc = 0xFFFF;
    for (std::size_t index = 0; index < size; ++index) {
        crc ^= bytes[index];
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc & 1U) != 0U ? static_cast<std::uint16_t>((crc >> 1U) ^ 0xA001U)
                                  : static_cast<std::uint16_t>(crc >> 1U);
        }
    }
    return crc;
}

void Rs485ModbusRtuClient::drainInput() {
    while (serial_.available() > 0) {
        (void)serial_.read();
    }
}

std::optional<std::uint16_t> Rs485ModbusRtuClient::readHolding(std::uint16_t address) {
    drainInput();
    std::array<std::uint8_t, 8> request{
        unitId_, 0x03,
        static_cast<std::uint8_t>(address >> 8U), static_cast<std::uint8_t>(address),
        0x00, 0x01, 0x00, 0x00,
    };
    const auto requestCrc = crc16(request.data(), 6);
    request[6] = static_cast<std::uint8_t>(requestCrc);
    request[7] = static_cast<std::uint8_t>(requestCrc >> 8U);

    digitalWrite(directionPin_, HIGH);
    serial_.write(request.data(), request.size());
    serial_.flush();
    digitalWrite(directionPin_, LOW);

    std::array<std::uint8_t, 7> response{};
    std::size_t received = 0;
    const auto deadline = millis() + timeoutMs_;
    while (received < response.size() && static_cast<std::int32_t>(millis() - deadline) < 0) {
        if (serial_.available() > 0) {
            response[received++] = static_cast<std::uint8_t>(serial_.read());
        }
        delay(1);
    }
    if (received != response.size() || response[0] != unitId_ || response[1] != 0x03 ||
        response[2] != 0x02) {
        return std::nullopt;
    }
    const auto responseCrc = static_cast<std::uint16_t>(response[5]) |
                             static_cast<std::uint16_t>(response[6] << 8U);
    if (crc16(response.data(), 5) != responseCrc) {
        return std::nullopt;
    }
    return static_cast<std::uint16_t>(response[3] << 8U) | response[4];
}

}  // namespace gridex::mbus

#endif
