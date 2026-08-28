#pragma once

#include "gridex/rockpie/MbusNodeTelemetry.hpp"
#include "gridex/rockpie/MbusRtuClient.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

namespace gridex::rockpie {

struct MbusPollingConfig {
    std::vector<std::uint8_t> addresses;
    std::chrono::milliseconds interval{500};
};

class MbusPollingService {
public:
    MbusPollingService(MbusRtuClient& client, MbusPollingConfig config);
    ~MbusPollingService();

    MbusPollingService(const MbusPollingService&) = delete;
    MbusPollingService& operator=(const MbusPollingService&) = delete;

    void start();
    void stop();
    [[nodiscard]] std::vector<MbusNodeTelemetry> samples() const;

private:
    MbusRtuClient& client_;
    MbusPollingConfig config_;
    mutable std::mutex mutex_;
    std::vector<MbusNodeTelemetry> samples_;
    std::atomic_bool running_{false};
    std::thread worker_;

    void run();
    void pollNode(std::size_t index);
};

}  // namespace gridex::rockpie
