#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace gridex::rockpie {

struct MbusSerialEndpoint {
    std::string device{"/dev/ttyS1"};
    std::uint32_t baud{115200};
    std::chrono::milliseconds timeout{80};
};

struct MbusNodeIdentity {
    std::uint8_t address{0};
    std::uint16_t mapVersion{0};
    std::uint16_t nodeType{0};
    std::uint16_t state{0};
    std::uint64_t uid{0};
};

class MbusRtuClient {
public:
    explicit MbusRtuClient(MbusSerialEndpoint endpoint);
    ~MbusRtuClient();

    MbusRtuClient(const MbusRtuClient&) = delete;
    MbusRtuClient& operator=(const MbusRtuClient&) = delete;

    [[nodiscard]] std::optional<MbusNodeIdentity> readIdentity(
        std::uint8_t address
    );
    [[nodiscard]] std::vector<MbusNodeIdentity> scan(
        std::uint8_t first = 200,
        std::uint8_t last = 247
    );
    [[nodiscard]] bool assign(
        std::uint8_t temporaryAddress,
        std::uint64_t expectedUid,
        std::uint8_t finalAddress,
        std::uint16_t nodeType
    );
    void closeDevice();

private:
    MbusSerialEndpoint endpoint_;
    int fd_{-1};

    bool openDevice();
    [[nodiscard]] std::optional<std::vector<std::uint8_t>> transact(
        const std::vector<std::uint8_t>& request,
        std::size_t expectedResponseSize
    );
};

}  // namespace gridex::rockpie
