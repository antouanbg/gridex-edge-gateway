#include "gridex/rockpie/MbusPollingService.hpp"

#include <algorithm>
#include <thread>
#include <utility>

namespace gridex::rockpie {

MbusPollingService::MbusPollingService(
    MbusRtuClient& client,
    MbusPollingConfig config
) : client_(client), config_(std::move(config)) {
    if (config_.interval < std::chrono::milliseconds(100)) {
        config_.interval = std::chrono::milliseconds(100);
    }
    std::sort(config_.addresses.begin(), config_.addresses.end());
    config_.addresses.erase(
        std::unique(config_.addresses.begin(), config_.addresses.end()),
        config_.addresses.end()
    );
    for (const auto address : config_.addresses) {
        samples_.push_back(MbusNodeTelemetry{.address = address});
    }
}

MbusPollingService::~MbusPollingService() {
    stop();
}

void MbusPollingService::start() {
    if (running_.exchange(true)) return;
    worker_ = std::thread(&MbusPollingService::run, this);
}

void MbusPollingService::stop() {
    if (!running_.exchange(false)) return;
    if (worker_.joinable()) worker_.join();
    client_.closeDevice();
}

std::vector<MbusNodeTelemetry> MbusPollingService::samples() const {
    std::scoped_lock lock(mutex_);
    return samples_;
}

void MbusPollingService::run() {
    while (running_) {
        const auto cycleStarted = std::chrono::steady_clock::now();
        for (std::size_t index = 0; index < config_.addresses.size() && running_;
             ++index) {
            pollNode(index);
        }
        while (running_ &&
               std::chrono::steady_clock::now() - cycleStarted <
                   config_.interval) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }
}

void MbusPollingService::pollNode(std::size_t index) {
    const auto address = config_.addresses[index];
    const auto identity = client_.readHolding(address, 0x0000U, 13U);
    const auto telemetry = client_.readHolding(address, 0x0040U, 6U);
    std::scoped_lock lock(mutex_);
    auto& sample = samples_[index];
    if (!identity || !telemetry || (*identity)[0] != 0x4758U) {
        sample.online = false;
        return;
    }
    sample.nodeType = (*identity)[2];
    sample.nodeState = (*identity)[3];
    sample.driverId = (*identity)[5];
    sample.quality = (*identity)[6];
    sample.heartbeat = (*identity)[12];
    sample.actualPowerKw = static_cast<double>(
        static_cast<std::int16_t>((*telemetry)[0])
    ) / 10.0;
    sample.energyWh =
        (static_cast<std::uint32_t>((*telemetry)[1]) << 16U) |
        (*telemetry)[2];
    sample.deviceState = (*telemetry)[3];
    sample.alarmBits = (*telemetry)[4];
    sample.cloudConnected = (*telemetry)[5] == 1U;
    sample.online = true;
    sample.lastSeen = std::chrono::steady_clock::now();
}

}  // namespace gridex::rockpie
