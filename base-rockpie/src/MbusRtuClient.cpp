#include "gridex/rockpie/MbusRtuClient.hpp"

#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

namespace gridex::rockpie {
namespace {

constexpr std::uint16_t kMagic = 0x4758U;
constexpr std::uint16_t kRequestedAddress = 0x0010U;
constexpr std::uint16_t kApplyKey = 0xA55AU;

void appendU16(std::vector<std::uint8_t>& bytes, std::uint16_t value) {
    bytes.push_back(static_cast<std::uint8_t>(value >> 8U));
    bytes.push_back(static_cast<std::uint8_t>(value));
}

std::uint16_t readU16(const std::uint8_t* bytes) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(bytes[0]) << 8U) | bytes[1]
    );
}

std::uint16_t crc16(const std::uint8_t* bytes, std::size_t size) {
    std::uint16_t crc = 0xFFFFU;
    for (std::size_t i = 0; i < size; ++i) {
        crc ^= bytes[i];
        for (int bit = 0; bit < 8; ++bit) {
            const bool lsb = (crc & 1U) != 0U;
            crc >>= 1U;
            if (lsb) {
                crc ^= 0xA001U;
            }
        }
    }
    return crc;
}

std::vector<std::uint8_t> finish(std::vector<std::uint8_t> frame) {
    const auto crc = crc16(frame.data(), frame.size());
    frame.push_back(static_cast<std::uint8_t>(crc));
    frame.push_back(static_cast<std::uint8_t>(crc >> 8U));
    return frame;
}

speed_t baudConstant(std::uint32_t baud) {
    return baud == 115200U ? B115200 : B9600;
}

}  // namespace

MbusRtuClient::MbusRtuClient(MbusSerialEndpoint endpoint)
    : endpoint_(std::move(endpoint)) {}

MbusRtuClient::~MbusRtuClient() {
    closeDevice();
}

bool MbusRtuClient::openDevice() {
    if (fd_ >= 0) {
        return true;
    }
    fd_ = ::open(endpoint_.device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd_ < 0) {
        return false;
    }

    termios options{};
    if (::tcgetattr(fd_, &options) != 0) {
        closeDevice();
        return false;
    }
    ::cfmakeraw(&options);
    const auto speed = baudConstant(endpoint_.baud);
    ::cfsetispeed(&options, speed);
    ::cfsetospeed(&options, speed);
    options.c_cflag |= CLOCAL | CREAD;
    options.c_cflag &= static_cast<tcflag_t>(~(PARENB | CSTOPB | CSIZE));
    options.c_cflag |= CS8;
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 0;
    if (::tcsetattr(fd_, TCSANOW, &options) != 0) {
        closeDevice();
        return false;
    }
    ::tcflush(fd_, TCIOFLUSH);
    return true;
}

void MbusRtuClient::closeDevice() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

std::optional<std::vector<std::uint8_t>> MbusRtuClient::transact(
    const std::vector<std::uint8_t>& request,
    std::size_t expectedResponseSize
) {
    if (!openDevice()) {
        return std::nullopt;
    }
    ::tcflush(fd_, TCIFLUSH);
    if (::write(fd_, request.data(), request.size()) !=
        static_cast<ssize_t>(request.size())) {
        closeDevice();
        return std::nullopt;
    }
    ::tcdrain(fd_);

    std::vector<std::uint8_t> response(expectedResponseSize);
    std::size_t received = 0;
    const auto deadline = std::chrono::steady_clock::now() + endpoint_.timeout;
    while (received < expectedResponseSize) {
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
            deadline - std::chrono::steady_clock::now()
        );
        if (remaining.count() <= 0) {
            return std::nullopt;
        }
        pollfd descriptor{fd_, POLLIN, 0};
        const int ready = ::poll(&descriptor, 1, static_cast<int>(remaining.count()));
        if (ready <= 0 || (descriptor.revents & POLLIN) == 0) {
            return std::nullopt;
        }
        const auto count = ::read(
            fd_,
            response.data() + received,
            expectedResponseSize - received
        );
        if (count <= 0) {
            return std::nullopt;
        }
        received += static_cast<std::size_t>(count);
    }

    const auto expectedCrc = crc16(response.data(), response.size() - 2U);
    const auto receivedCrc = static_cast<std::uint16_t>(
        response[response.size() - 2U] |
        (static_cast<std::uint16_t>(response.back()) << 8U)
    );
    return expectedCrc == receivedCrc
               ? std::optional<std::vector<std::uint8_t>>(std::move(response))
               : std::nullopt;
}

std::optional<MbusNodeIdentity> MbusRtuClient::readIdentity(
    std::uint8_t address
) {
    auto request = finish({address, 0x03U, 0x00U, 0x00U, 0x00U, 0x0DU});
    const auto response = transact(request, 31U);
    if (!response || (*response)[0] != address || (*response)[1] != 0x03U ||
        (*response)[2] != 26U || readU16(response->data() + 3) != kMagic) {
        return std::nullopt;
    }
    MbusNodeIdentity identity;
    identity.address = address;
    identity.mapVersion = readU16(response->data() + 5);
    identity.nodeType = readU16(response->data() + 7);
    identity.state = readU16(response->data() + 9);
    identity.driverId = readU16(response->data() + 13);
    identity.uid =
        (static_cast<std::uint64_t>(readU16(response->data() + 17)) << 48U) |
        (static_cast<std::uint64_t>(readU16(response->data() + 19)) << 32U) |
        (static_cast<std::uint64_t>(readU16(response->data() + 21)) << 16U) |
        readU16(response->data() + 23);
    return identity;
}

std::vector<MbusNodeIdentity> MbusRtuClient::scan(
    std::uint8_t first,
    std::uint8_t last
) {
    std::vector<MbusNodeIdentity> nodes;
    for (std::uint16_t address = first; address <= last; ++address) {
        if (auto identity = readIdentity(static_cast<std::uint8_t>(address))) {
            nodes.push_back(*identity);
        }
    }
    return nodes;
}

bool MbusRtuClient::assign(
    std::uint8_t temporaryAddress,
    std::uint64_t expectedUid,
    std::uint8_t finalAddress,
    std::uint16_t nodeType,
    std::uint16_t driverId
) {
    const auto identity = readIdentity(temporaryAddress);
    if (!identity || identity->uid != expectedUid || finalAddress == 0U ||
        finalAddress > 247U || driverId == 0U) {
        return false;
    }
    std::vector<std::uint8_t> request{temporaryAddress, 0x10U};
    appendU16(request, kRequestedAddress);
    appendU16(request, 4U);
    request.push_back(8U);
    appendU16(request, finalAddress);
    appendU16(request, nodeType);
    appendU16(request, driverId);
    appendU16(request, kApplyKey);
    request = finish(std::move(request));
    const auto response = transact(request, 8U);
    return response && (*response)[0] == temporaryAddress &&
           (*response)[1] == 0x10U;
}

}  // namespace gridex::rockpie
