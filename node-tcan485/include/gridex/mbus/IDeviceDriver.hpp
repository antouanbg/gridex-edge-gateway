#pragma once

#include "gridex/mbus/RegisterMap.hpp"

#include <cstdint>

namespace gridex::mbus {

struct DriverSample {
    std::int16_t actualPowerKwX10{0};
    std::uint32_t energyWh{0};
    std::uint16_t state{0};
    std::uint16_t alarms{0};
    std::uint16_t quality{0};
};

class IDeviceDriver {
public:
    virtual ~IDeviceDriver() = default;
    virtual bool begin() = 0;
    virtual DriverSample poll() = 0;
    virtual bool applyPowerCommand(std::int16_t powerKwX10) = 0;
};

class UnconfiguredDriver final : public IDeviceDriver {
public:
    explicit UnconfiguredDriver(NodeType type) : type_(type) {}

    bool begin() override { return false; }
    DriverSample poll() override {
        DriverSample sample;
        sample.state = static_cast<std::uint16_t>(type_);
        sample.quality = 0;
        return sample;
    }
    bool applyPowerCommand(std::int16_t) override { return false; }

private:
    NodeType type_;
};

}  // namespace gridex::mbus
