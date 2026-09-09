#include "gridex/rockpie/NodeTcpPollingService.hpp"

#include <algorithm>
#include <cmath>
#include <thread>
#include <utility>

namespace gridex::rockpie {
namespace {

constexpr std::uint16_t kMagic = 0x4758U;
constexpr std::uint16_t kIdentityStart = 0x0000U;
constexpr std::uint16_t kIdentityCount = 13U;
constexpr std::uint16_t kCommandStart = 0x0014U;
constexpr std::uint16_t kTelemetryStart = 0x0040U;
constexpr std::uint16_t kTelemetryCount = 6U;

}  // namespace

NodeTcpPollingService::NodeTcpPollingService(NodeTcpPollingConfig config)
    : config_(std::move(config)) {
    if (config_.interval < std::chrono::milliseconds(100)) {
        config_.interval = std::chrono::milliseconds(100);
    }
    for (std::size_t index = 0; index < config_.endpoints.size(); ++index) {
        const auto& endpoint = config_.endpoints[index];
        clients_.push_back(std::make_unique<PosixModbusTcpClient>(
            ModbusTcpEndpoint{
                .host = endpoint.host,
                .port = endpoint.port,
                .unitId = endpoint.unitId,
                .timeout = config_.timeout,
            }
        ));
        samples_.push_back(MbusNodeTelemetry{
            .address = static_cast<std::uint8_t>(index + 1U)
        });
    }
}

NodeTcpPollingService::~NodeTcpPollingService() {
    stop();
}

void NodeTcpPollingService::start() {
    if (running_.exchange(true)) return;
    worker_ = std::thread(&NodeTcpPollingService::run, this);
}

void NodeTcpPollingService::stop() {
    if (!running_.exchange(false)) return;
    if (worker_.joinable()) worker_.join();
    for (auto& client : clients_) client->disconnect();
}

std::vector<MbusNodeTelemetry> NodeTcpPollingService::samples() const {
    std::scoped_lock lock(mutex_);
    return samples_;
}

void NodeTcpPollingService::run() {
    while (running_) {
        const auto cycleStarted = std::chrono::steady_clock::now();
        for (std::size_t index = 0; index < clients_.size() && running_;
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

void NodeTcpPollingService::pollNode(std::size_t index) {
    const auto identity =
        clients_[index]->readHoldingRange(kIdentityStart, kIdentityCount);
    const auto telemetry =
        clients_[index]->readHoldingRange(kTelemetryStart, kTelemetryCount);
    std::scoped_lock lock(mutex_);
    auto& sample = samples_[index];
    if (!identity || !telemetry || (*identity)[0] != kMagic) {
        sample.online = false;
        return;
    }
    sample.nodeType = (*identity)[2];
    sample.nodeState = (*identity)[3];
    sample.driverId = (*identity)[5];
    sample.quality = (*identity)[6];
    sample.heartbeat = (*identity)[12];
    sample.actualPowerKw =
        static_cast<double>(static_cast<std::int16_t>((*telemetry)[0])) / 10.0;
    sample.energyWh =
        (static_cast<std::uint32_t>((*telemetry)[1]) << 16U) |
        (*telemetry)[2];
    sample.deviceState = (*telemetry)[3];
    sample.alarmBits = (*telemetry)[4];
    sample.cloudConnected = (*telemetry)[5] == 1U;
    sample.online = true;
    sample.lastSeen = std::chrono::steady_clock::now();
}

bool NodeTcpPollingService::applyPowerCommand(
    std::size_t slot,
    double requestedPowerKw,
    bool enabled,
    std::uint16_t sequence,
    std::uint16_t ttlSeconds
) {
    if (slot >= clients_.size() || !std::isfinite(requestedPowerKw) ||
        requestedPowerKw < -3276.8 || requestedPowerKw > 3276.7 ||
        ttlSeconds < 1U || ttlSeconds > 30U) {
        return false;
    }
    const auto raw = static_cast<std::int16_t>(
        std::lround(requestedPowerKw * 10.0)
    );
    return clients_[slot]->writeHoldingRange(
        kCommandStart,
        {
            sequence,
            static_cast<std::uint16_t>(raw),
            static_cast<std::uint16_t>(enabled ? 1U : 0U),
            ttlSeconds,
        }
    );
}

}  // namespace gridex::rockpie
