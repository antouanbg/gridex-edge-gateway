#pragma once

#include "gridex/IModbusClient.hpp"

#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace gridex::rockpie {

struct ModbusTcpEndpoint {
    std::string host{"192.168.50.10"};
    std::uint16_t port{3200};
    std::uint8_t unitId{1};
    std::chrono::milliseconds timeout{800};
};

class PosixModbusTcpClient final : public IModbusClient {
public:
    explicit PosixModbusTcpClient(ModbusTcpEndpoint endpoint);
    ~PosixModbusTcpClient() override;

    PosixModbusTcpClient(const PosixModbusTcpClient&) = delete;
    PosixModbusTcpClient& operator=(const PosixModbusTcpClient&) = delete;

    std::optional<std::uint16_t> readInput(std::uint16_t address) override;
    std::optional<std::uint16_t> readHolding(std::uint16_t address) override;
    std::optional<bool> readCoil(std::uint16_t address) override;
    bool writeHolding(std::uint16_t address, std::uint16_t value) override;

    void disconnect();

private:
    ModbusTcpEndpoint endpoint_;
    int socket_{-1};
    std::uint16_t transactionId_{0};
    std::mutex mutex_;

    bool connectLocked();
    void disconnectLocked();
    std::optional<std::vector<std::uint8_t>> transactLocked(
        std::uint8_t function,
        const std::vector<std::uint8_t>& payload
    );
    std::optional<std::uint16_t> readRegister(
        std::uint8_t function,
        std::uint16_t address
    );
};

}  // namespace gridex::rockpie
