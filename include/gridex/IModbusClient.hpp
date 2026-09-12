#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace gridex {

class IModbusClient {
public:
    virtual ~IModbusClient() = default;

    virtual std::optional<std::uint16_t> readInput(std::uint16_t address) = 0;
    // A contiguous read is required for values that the device updates as one
    // snapshot, such as the Suntech 32-bit energy counters at 122-125.
    virtual std::optional<std::vector<std::uint16_t>> readInputRange(
        std::uint16_t start,
        std::uint16_t count
    ) = 0;
    virtual std::optional<std::uint16_t> readHolding(std::uint16_t address) = 0;
    virtual std::optional<bool> readCoil(std::uint16_t address) = 0;
    virtual bool writeHolding(std::uint16_t address, std::uint16_t value) = 0;
};

}  // namespace gridex
