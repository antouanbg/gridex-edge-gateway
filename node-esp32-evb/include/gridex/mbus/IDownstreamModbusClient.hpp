#pragma once

#include <cstdint>
#include <optional>

namespace gridex::mbus {

// Vendor drivers use the optional isolated UEXT/UART RS485 transceiver.
// The canonical northbound path to ROCK Pi E is Ethernet, not this bus.
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
