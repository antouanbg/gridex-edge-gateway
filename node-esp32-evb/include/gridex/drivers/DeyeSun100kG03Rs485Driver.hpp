#pragma once

#ifdef ARDUINO

#include "gridex/mbus/IDeviceDriver.hpp"
#include "gridex/mbus/Rs485ModbusRtuClient.hpp"

namespace gridex::drivers {

// Deye SUN-100K-G03 string inverter telemetry profile. The public V118 map
// confirms reads below; it does not confirm an active-power limit write, so
// applyPowerCommand is deliberately locked.
class DeyeSun100kG03Rs485Driver final : public mbus::IDeviceDriver {
public:
    explicit DeyeSun100kG03Rs485Driver(mbus::Rs485ModbusRtuClient& transport)
        : transport_(transport) {}

    bool begin() override;
    mbus::DriverSample poll() override;
    bool applyPowerCommand(std::int16_t powerKwX10) override;

private:
    mbus::Rs485ModbusRtuClient& transport_;
    bool online_{false};

    [[nodiscard]] static std::uint32_t joinU32(std::uint16_t high, std::uint16_t low);
};

}  // namespace gridex::drivers

#endif
