#include "gridex/rockpie/NorthboundModbusTcpServer.hpp"

#include <arpa/inet.h>
#include <array>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <poll.h>
#include <span>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

namespace gridex::rockpie {
namespace {

std::uint16_t readU16(std::span<const std::uint8_t> bytes, std::size_t offset) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(bytes[offset]) << 8U) |
        bytes[offset + 1U]
    );
}

void appendU16(std::vector<std::uint8_t>& bytes, std::uint16_t value) {
    bytes.push_back(static_cast<std::uint8_t>(value >> 8U));
    bytes.push_back(static_cast<std::uint8_t>(value & 0xFFU));
}

bool receiveExact(int fd, std::span<std::uint8_t> target) {
    std::size_t received = 0U;
    while (received < target.size()) {
        const auto result = ::recv(
            fd,
            target.data() + received,
            target.size() - received,
            0
        );
        if (result == 0) return false;
        if (result < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        received += static_cast<std::size_t>(result);
    }
    return true;
}

bool sendAll(int fd, std::span<const std::uint8_t> data) {
    std::size_t sent = 0U;
    while (sent < data.size()) {
#ifdef MSG_NOSIGNAL
        constexpr int sendFlags = MSG_NOSIGNAL;
#else
        constexpr int sendFlags = 0;
#endif
        const auto result = ::send(
            fd,
            data.data() + sent,
            data.size() - sent,
            sendFlags
        );
        if (result < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        sent += static_cast<std::size_t>(result);
    }
    return true;
}

std::vector<std::uint8_t> exception(std::uint8_t function, std::uint8_t code) {
    return {static_cast<std::uint8_t>(function | 0x80U), code};
}

}  // namespace

NorthboundModbusTcpServer::NorthboundModbusTcpServer(
    NorthboundRegisterBank& bank,
    NorthboundServerConfig config
) : bank_(bank), config_(std::move(config)) {}

NorthboundModbusTcpServer::~NorthboundModbusTcpServer() {
    stop();
}

bool NorthboundModbusTcpServer::start() {
    if (running_) return true;

    listenFd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ < 0) return false;

    int reuse = 1;
    ::setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(config_.port);
    if (::inet_pton(AF_INET, config_.bindAddress.c_str(), &address.sin_addr) != 1 ||
        ::bind(
            listenFd_,
            reinterpret_cast<const sockaddr*>(&address),
            sizeof(address)
        ) != 0 ||
        ::listen(listenFd_, 4) != 0) {
        ::close(listenFd_);
        listenFd_ = -1;
        return false;
    }

    running_ = true;
    worker_ = std::thread(&NorthboundModbusTcpServer::run, this);
    return true;
}

void NorthboundModbusTcpServer::stop() {
    if (!running_.exchange(false)) return;
    if (listenFd_ >= 0) {
        ::shutdown(listenFd_, SHUT_RDWR);
        ::close(listenFd_);
        listenFd_ = -1;
    }
    if (worker_.joinable()) worker_.join();
}

bool NorthboundModbusTcpServer::running() const {
    return running_;
}

void NorthboundModbusTcpServer::run() {
    while (running_) {
        pollfd descriptor{.fd = listenFd_, .events = POLLIN, .revents = 0};
        const int ready = ::poll(&descriptor, 1, 250);
        if (!running_) break;
        if (ready <= 0 || (descriptor.revents & POLLIN) == 0) continue;

        const int clientFd = ::accept(listenFd_, nullptr, nullptr);
        if (clientFd < 0) continue;
        handleClient(clientFd);
        ::close(clientFd);
    }
}

void NorthboundModbusTcpServer::handleClient(int clientFd) {
    while (running_) {
        pollfd descriptor{.fd = clientFd, .events = POLLIN, .revents = 0};
        const int ready = ::poll(&descriptor, 1, 250);
        if (ready < 0) return;
        if (ready == 0) continue;
        if ((descriptor.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) return;
        if ((descriptor.revents & POLLIN) == 0) continue;

        std::array<std::uint8_t, 7> header{};
        if (!receiveExact(clientFd, header)) return;
        const auto protocolId = readU16(header, 2);
        const auto length = readU16(header, 4);
        if (protocolId != 0U || length < 2U || length > 254U) return;

        std::vector<std::uint8_t> pdu(length - 1U);
        if (!receiveExact(clientFd, pdu)) return;

        std::vector<std::uint8_t> responsePdu;
        if (header[6] != config_.unitId) {
            responsePdu = exception(pdu[0], 0x0BU);
        } else {
            responsePdu = handlePdu(pdu);
        }

        std::vector<std::uint8_t> response;
        response.reserve(7U + responsePdu.size());
        response.insert(response.end(), header.begin(), header.begin() + 4);
        appendU16(
            response,
            static_cast<std::uint16_t>(responsePdu.size() + 1U)
        );
        response.push_back(header[6]);
        response.insert(response.end(), responsePdu.begin(), responsePdu.end());
        if (!sendAll(clientFd, response)) return;
    }
}

std::vector<std::uint8_t> NorthboundModbusTcpServer::handlePdu(
    std::span<const std::uint8_t> pdu
) {
    if (pdu.empty()) return exception(0U, 0x03U);
    const auto function = pdu[0];

    if (function == 0x03U || function == 0x04U) {
        if (pdu.size() != 5U) return exception(function, 0x03U);
        const auto start = readU16(pdu, 1);
        const auto count = readU16(pdu, 3);
        if (count == 0U || count > 125U) return exception(function, 0x03U);
        const auto values = function == 0x03U
            ? bank_.readHolding(start, count)
            : bank_.readInput(start, count);
        if (!values) return exception(function, 0x02U);

        std::vector<std::uint8_t> response{
            function,
            static_cast<std::uint8_t>(values->size() * 2U),
        };
        for (const auto value : *values) appendU16(response, value);
        return response;
    }

    if (function == 0x06U) {
        if (pdu.size() != 5U) return exception(function, 0x03U);
        const auto address = readU16(pdu, 1);
        const auto value = readU16(pdu, 3);
        if (!bank_.writeHolding(address, std::span(&value, 1U))) {
            return exception(function, 0x02U);
        }
        return std::vector<std::uint8_t>(pdu.begin(), pdu.end());
    }

    if (function == 0x10U) {
        if (pdu.size() < 6U) return exception(function, 0x03U);
        const auto start = readU16(pdu, 1);
        const auto count = readU16(pdu, 3);
        const auto byteCount = pdu[5];
        if (count == 0U || count > 123U || byteCount != count * 2U ||
            pdu.size() != static_cast<std::size_t>(6U + byteCount)) {
            return exception(function, 0x03U);
        }
        std::vector<std::uint16_t> values;
        values.reserve(count);
        for (std::uint16_t index = 0; index < count; ++index) {
            values.push_back(readU16(pdu, 6U + index * 2U));
        }
        if (!bank_.writeHolding(start, values)) {
            return exception(function, 0x02U);
        }
        std::vector<std::uint8_t> response{function};
        appendU16(response, start);
        appendU16(response, count);
        return response;
    }

    return exception(function, 0x01U);
}

}  // namespace gridex::rockpie
