#pragma once

#include "gridex/IModbusClient.hpp"
#include "gridex/Types.hpp"

#include <cstdint>

namespace gridex {

struct CommissioningApproval {
    bool addressingConfirmed{false};
    bool signConfirmed{false};
    bool scalingConfirmed{false};

    [[nodiscard]] bool writesEnabled() const {
        return addressingConfirmed && signConfirmed && scalingConfirmed;
    }
};

class SunStoragePro261Driver {
public:
    struct Registers {
        static constexpr std::uint16_t PcsFault = 1;
        static constexpr std::uint16_t PcsOffGrid = 2;
        static constexpr std::uint16_t PcsCommunicationFault = 4;
        static constexpr std::uint16_t PcsActivePower = 9;
        static constexpr std::uint16_t PcsStatus = 16;
        static constexpr std::uint16_t BmsCurrent = 101;
        static constexpr std::uint16_t BmsSoc = 102;
        static constexpr std::uint16_t BmsSoh = 103;
        static constexpr std::uint16_t BmsVoltage = 104;
        static constexpr std::uint16_t BmsStatus = 105;
        static constexpr std::uint16_t BmsSystemFlags = 126;
        static constexpr std::uint16_t BmsMaxChargePower = 127;
        static constexpr std::uint16_t BmsMaxDischargePower = 128;
        static constexpr std::uint16_t BmsFault = 184;
        static constexpr std::uint16_t BmsCommunicationFault = 185;
        static constexpr std::uint16_t PcsGridMode = 5001;
        static constexpr std::uint16_t PcsWorkMode = 5002;
        static constexpr std::uint16_t PcsPowerOn = 5003;
        static constexpr std::uint16_t PcsPowerCommand = 5005;
        static constexpr std::uint16_t PcsReactivePowerCommand = 5006;
        static constexpr std::uint16_t HeartbeatSeconds = 5301;
        static constexpr std::uint16_t HeartbeatEnable = 5302;
    };

    explicit SunStoragePro261Driver(
        IModbusClient& client,
        CommissioningApproval approval = {}
    );

    [[nodiscard]] BatteryTelemetry poll();
    [[nodiscard]] bool refreshHeartbeat(std::uint16_t timeoutSeconds = 60);
    [[nodiscard]] bool writePowerSetpointKw(double canonicalPowerKw);
    [[nodiscard]] bool writeReactivePowerSetpointKvar(double reactivePowerKvar);
    [[nodiscard]] bool writesEnabled() const;
    [[nodiscard]] bool controlReady() const;

private:
    IModbusClient& client_;
    CommissioningApproval approval_;
    bool controlReady_{false};
    bool limitsValid_{false};
    double maxChargeKw_{0.0};
    double maxDischargeKw_{0.0};
    std::uint16_t bmsSystemFlags_{0};

    [[nodiscard]] static double decodeSigned(std::uint16_t raw, double scale);
    [[nodiscard]] static std::uint16_t encodeSigned(double value, double scale);
};

}  // namespace gridex
