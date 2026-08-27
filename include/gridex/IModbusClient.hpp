#pragma once

#include <cstdint>
#include <optional>

namespace gridex {

class IModbusClient {
public:
    virtual ~IModbusClient() = default;

    virtual std::optional<std::uint16_t> readInput(std::uint16_t address) = 0;
    virtual std::optional<std::uint16_t> readHolding(std::uint16_t address) = 0;
    virtual std::optional<bool> readCoil(std::uint16_t address) = 0;
    virtual bool writeHolding(std::uint16_t address, std::uint16_t value) = 0;
};

}  // namespace gridex

