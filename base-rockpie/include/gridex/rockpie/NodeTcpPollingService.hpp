#pragma once

#include "gridex/rockpie/MbusNodeTelemetry.hpp"
#include "gridex/rockpie/PosixModbusTcpClient.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace gridex::rockpie {

struct NodeTcpEndpoint {
    std::string host;
    std::uint16_t port{1502};
    std::uint8_t unitId{1};
};

struct NodeTcpPollingConfig {
    std::vector<NodeTcpEndpoint> endpoints;
    std::chrono::milliseconds interval{500};
    std::chrono::milliseconds timeout{400};
};

class NodeTcpPollingService {
public:
    explicit NodeTcpPollingService(NodeTcpPollingConfig config);
    ~NodeTcpPollingService();

    NodeTcpPollingService(const NodeTcpPollingService&) = delete;
    NodeTcpPollingService& operator=(const NodeTcpPollingService&) = delete;

    void start();
    void stop();
    [[nodiscard]] std::vector<MbusNodeTelemetry> samples() const;
    [[nodiscard]] bool applyPowerCommand(
        std::size_t slot,
        double requestedPowerKw,
        bool enabled,
        std::uint16_t sequence,
        std::uint16_t ttlSeconds
    );

private:
    NodeTcpPollingConfig config_;
    std::vector<std::unique_ptr<PosixModbusTcpClient>> clients_;
    mutable std::mutex mutex_;
    std::vector<MbusNodeTelemetry> samples_;
    std::atomic_bool running_{false};
    std::thread worker_;

    void run();
    void pollNode(std::size_t index);
};

}  // namespace gridex::rockpie
