#pragma once

#ifdef ARDUINO

#include "gridex/mbus/IDeviceDriver.hpp"
#include "gridex/mbus/Rs485ModbusRtuClient.hpp"

namespace gridex::drivers {

struct DeyeSun100kG03ControlConfig {
    bool writesEnabled{false};
    bool requireControlEnableRegister{false};
    std::uint16_t maximumRegulationTenthsPct{1000};
};

// Deye SUN-100K-G03 telemetry and active-power limiter. Command kW values are
// converted to the vendor's 0.1% holding-register 77 representation.
class DeyeSun100kG03Rs485Driver final : public mbus::IDeviceDriver {
public:
    explicit DeyeSun100kG03Rs485Driver(
        mbus::Rs485ModbusRtuClient& transport,
        DeyeSun100kG03ControlConfig config = {}
    ) : transport_(transport), config_(config) {}

    bool begin() override;
    mbus::DriverSample poll() override;
    bool applyPowerCommand(std::int16_t powerKwX10) override;

private:
    mbus::Rs485ModbusRtuClient& transport_;
    DeyeSun100kG03ControlConfig config_;
    bool online_{false};
    std::uint16_t ratedPowerKwX10_{0};

    [[nodiscard]] static std::uint32_t joinU32(std::uint16_t high, std::uint16_t low);
};

}  // namespace gridex::drivers

#endif
