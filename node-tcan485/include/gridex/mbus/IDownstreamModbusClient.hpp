#pragma once

#include <cstdint>
#include <optional>

namespace gridex::mbus {

// Vendor drivers use this second, electrically separate RS485 channel.
// The onboard T-CAN485 transceiver remains dedicated to upstream MBUS.
class IDownstreamModbusClient {
public:
    virtual ~IDownstreamModbusClient() = default;

    virtual std::optional<std::uint16_t> readInput(
        std::uint8_t unitId,
        std::uint16_t address
    ) = 0;
    virtual std::optional<std::uint16_t> readHolding(
        std::uint8_t unitId,
        std::uint16_t address
    ) = 0;
    virtual bool writeHolding(
        std::uint8_t unitId,
        std::uint16_t address,
        std::uint16_t value
    ) = 0;
};

struct DownstreamSerialProfile {
    std::uint32_t baud{9600};
    std::uint8_t dataBits{8};
    char parity{'N'};
    std::uint8_t stopBits{1};
    std::uint8_t unitId{1};
};

}  // namespace gridex::mbus
