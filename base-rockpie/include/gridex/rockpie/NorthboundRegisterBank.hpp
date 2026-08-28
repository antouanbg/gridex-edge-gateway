#pragma once

#include "gridex/EdgeController.hpp"
#include "gridex/NorthboundMap.hpp"
#include "gridex/rockpie/MbusNodeTelemetry.hpp"

#include <array>
#include <cstdint>
#include <mutex>
#include <optional>
#include <span>
#include <vector>

namespace gridex::rockpie {

struct NorthboundCommand {
    std::uint16_t sequence{};
    std::uint16_t heartbeat{};
    double requestedPowerKw{};
    bool enabled{false};
};

class NorthboundRegisterBank {
public:
    [[nodiscard]] std::optional<std::vector<std::uint16_t>> readHolding(
        std::uint16_t start,
        std::uint16_t count
    ) const;
    [[nodiscard]] std::optional<std::vector<std::uint16_t>> readInput(
        std::uint16_t start,
        std::uint16_t count
    ) const;
    [[nodiscard]] bool writeHolding(
        std::uint16_t start,
        std::span<const std::uint16_t> values
    );
    [[nodiscard]] std::optional<NorthboundCommand> takeCommand();
    void publish(
        const ControllerSnapshot& snapshot,
        const ConfiguredPowerLimit& configuredLimit
    );
    void publishNode(std::size_t slot, const MbusNodeTelemetry& sample);

private:
    mutable std::mutex mutex_;
    std::array<std::uint16_t, northbound::HoldingRegisterCount> holding_{};
    std::array<std::uint16_t, northbound::InputRegisterCount> input_{};
    bool commandObserved_{true};
    std::uint16_t lastSequence_{};
    std::uint16_t lastHeartbeat_{};
};

}  // namespace gridex::rockpie
