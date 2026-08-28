#pragma once

#include "gridex/rockpie/NorthboundRegisterBank.hpp"

#include <atomic>
#include <cstdint>
#include <span>
#include <string>
#include <thread>
#include <vector>

namespace gridex::rockpie {

struct NorthboundServerConfig {
    std::string bindAddress{"0.0.0.0"};
    std::uint16_t port{1502};
    std::uint8_t unitId{1};
};

class NorthboundModbusTcpServer {
public:
    NorthboundModbusTcpServer(
        NorthboundRegisterBank& bank,
        NorthboundServerConfig config = {}
    );
    ~NorthboundModbusTcpServer();

    NorthboundModbusTcpServer(const NorthboundModbusTcpServer&) = delete;
    NorthboundModbusTcpServer& operator=(const NorthboundModbusTcpServer&) = delete;

    [[nodiscard]] bool start();
    void stop();
    [[nodiscard]] bool running() const;

private:
    NorthboundRegisterBank& bank_;
    NorthboundServerConfig config_;
    std::atomic_bool running_{false};
    int listenFd_{-1};
    std::thread worker_;

    void run();
    void handleClient(int clientFd);
    [[nodiscard]] std::vector<std::uint8_t> handlePdu(
        std::span<const std::uint8_t> pdu
    );
};

}  // namespace gridex::rockpie
