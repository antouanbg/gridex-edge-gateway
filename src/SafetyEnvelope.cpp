#include "gridex/SafetyEnvelope.hpp"

#include <algorithm>
#include <cmath>

namespace gridex {

SafetyResult SafetyEnvelope::apply(const SafetyInput& input) const {
    SafetyResult result{
        .requestedPowerKw = input.requestedPowerKw,
        .appliedPowerKw = 0.0,
        .clamped = false,
        .reason = "safe",
    };

    if (!input.emsConnected) {
        result.clamped = std::abs(input.requestedPowerKw) > 0.001;
        result.reason = "EMS connection stale or missing";
        return result;
    }

    if (input.battery.quality != Quality::Good) {
        result.clamped = std::abs(input.requestedPowerKw) > 0.001;
        result.reason = "vendor telemetry is not valid";
        return result;
    }

    if (!input.battery.limitsValid) {
        result.clamped = std::abs(input.requestedPowerKw) > 0.001;
        result.reason = "BMS charge/discharge limits are not valid";
        return result;
    }

    const bool chargeProhibited = (input.battery.alarmBits & 0x0001U) != 0U;
    const bool dischargeProhibited = (input.battery.alarmBits & 0x0002U) != 0U;

    double maxChargeKw = std::max(0.0, input.battery.maxChargeKw);
    const double maxDischargeKw = std::max(0.0, input.battery.maxDischargeKw);

    if (input.site.siteLoadKw && input.site.contractLimitKw) {
        const double headroomKw =
            std::max(0.0, *input.site.contractLimitKw - *input.site.siteLoadKw);
        maxChargeKw = std::min(maxChargeKw, headroomKw);
    }

    double lowerBoundKw = chargeProhibited ? 0.0 : -maxChargeKw;
    double upperBoundKw = dischargeProhibited ? 0.0 : maxDischargeKw;
    result.appliedPowerKw =
        std::clamp(input.requestedPowerKw, lowerBoundKw, upperBoundKw);
    result.clamped =
        std::abs(result.appliedPowerKw - result.requestedPowerKw) > 0.001;

    if (chargeProhibited && input.requestedPowerKw < 0.0) {
        result.reason = "BMS charge prohibited";
    } else if (dischargeProhibited && input.requestedPowerKw > 0.0) {
        result.reason = "BMS discharge prohibited";
    } else if (result.clamped && input.requestedPowerKw < lowerBoundKw) {
        result.reason = "charge command clamped by BMS/software fuse";
    } else if (result.clamped && input.requestedPowerKw > upperBoundKw) {
        result.reason = "discharge command clamped by BMS limit";
    }

    return result;
}

}  // namespace gridex

