#pragma once

#include "gridex/mbus/RegisterMap.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gridex::mbus {

struct NodeConfig {
    std::uint8_t address{0};
    NodeType type{NodeType::Unconfigured};
    std::uint64_t uid{0};
};

class MbusNode {
public:
    explicit MbusNode(NodeConfig config);

    [[nodiscard]] std::vector<std::uint8_t> processFrame(
        const std::uint8_t* frame,
        std::size_t size
    );
    [[nodiscard]] std::uint8_t address() const;
    [[nodiscard]] NodeType type() const;
    [[nodiscard]] std::uint64_t uid() const;
    [[nodiscard]] bool takeConfigurationChanged();

    void setRegister(std::uint16_t address, std::uint16_t value);
    [[nodiscard]] std::uint16_t registerValue(std::uint16_t address) const;

    static std::uint16_t crc16(const std::uint8_t* bytes, std::size_t size);

private:
    NodeConfig config_;
    std::array<std::uint16_t, reg::HoldingCount> registers_{};
    bool configurationChanged_{false};

    [[nodiscard]] static std::uint8_t temporaryAddress(std::uint64_t uid);
    void initializeIdentity();
    void applyPendingConfiguration();
    [[nodiscard]] std::vector<std::uint8_t> exception(
        std::uint8_t requestAddress,
        std::uint8_t function,
        std::uint8_t code
    ) const;
    [[nodiscard]] static std::vector<std::uint8_t> withCrc(
        std::vector<std::uint8_t> response
    );
};

}  // namespace gridex::mbus
