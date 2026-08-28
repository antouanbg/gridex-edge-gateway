#include "gridex/rockpie/PosixModbusTcpClient.hpp"

#include <array>
#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

namespace gridex::rockpie {
namespace {

void appendU16(std::vector<std::uint8_t>& bytes, std::uint16_t value) {
    bytes.push_back(static_cast<std::uint8_t>(value >> 8U));
    bytes.push_back(static_cast<std::uint8_t>(value & 0xFFU));
}

std::uint16_t readU16(const std::uint8_t* bytes) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(bytes[0]) << 8U) | bytes[1]
    );
}

bool sendAll(int socket, const std::uint8_t* bytes, std::size_t size) {
    std::size_t sent = 0;
    while (sent < size) {
#ifdef MSG_NOSIGNAL
        const auto result = ::send(socket, bytes + sent, size - sent, MSG_NOSIGNAL);
#else
        const auto result = ::send(socket, bytes + sent, size - sent, 0);
#endif
        if (result <= 0) {
            return false;
        }
        sent += static_cast<std::size_t>(result);
    }
    return true;
}

bool receiveAll(int socket, std::uint8_t* bytes, std::size_t size) {
    std::size_t received = 0;
    while (received < size) {
        const auto result = ::recv(socket, bytes + received, size - received, 0);
        if (result <= 0) {
            return false;
        }
        received += static_cast<std::size_t>(result);
    }
    return true;
}

}  // namespace

PosixModbusTcpClient::PosixModbusTcpClient(ModbusTcpEndpoint endpoint)
    : endpoint_(std::move(endpoint)) {}

PosixModbusTcpClient::~PosixModbusTcpClient() {
    disconnect();
}

bool PosixModbusTcpClient::connectLocked() {
    if (socket_ >= 0) {
        return true;
    }

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* addresses = nullptr;
    const auto port = std::to_string(endpoint_.port);
    if (::getaddrinfo(endpoint_.host.c_str(), port.c_str(), &hints, &addresses) != 0) {
        return false;
    }

    for (auto* current = addresses; current != nullptr; current = current->ai_next) {
        const int candidate = ::socket(
            current->ai_family,
            current->ai_socktype,
            current->ai_protocol
        );
        if (candidate < 0) {
            continue;
        }

        timeval timeout{};
        timeout.tv_sec = static_cast<long>(endpoint_.timeout.count() / 1000);
        timeout.tv_usec = static_cast<long>((endpoint_.timeout.count() % 1000) * 1000);
        ::setsockopt(candidate, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        ::setsockopt(candidate, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

        if (::connect(candidate, current->ai_addr, current->ai_addrlen) == 0) {
            socket_ = candidate;
            break;
        }
        ::close(candidate);
    }
    ::freeaddrinfo(addresses);
    return socket_ >= 0;
}

void PosixModbusTcpClient::disconnectLocked() {
    if (socket_ >= 0) {
        ::close(socket_);
        socket_ = -1;
    }
}

void PosixModbusTcpClient::disconnect() {
    std::scoped_lock lock(mutex_);
    disconnectLocked();
}

std::optional<std::vector<std::uint8_t>> PosixModbusTcpClient::transactLocked(
    std::uint8_t function,
    const std::vector<std::uint8_t>& payload
) {
    if (!connectLocked()) {
        return std::nullopt;
    }

    const std::uint16_t transaction = ++transactionId_;
    std::vector<std::uint8_t> request;
    request.reserve(8U + payload.size());
    appendU16(request, transaction);
    appendU16(request, 0U);
    appendU16(
        request,
        static_cast<std::uint16_t>(2U + payload.size())
    );
    request.push_back(endpoint_.unitId);
    request.push_back(function);
    request.insert(request.end(), payload.begin(), payload.end());

    if (!sendAll(socket_, request.data(), request.size())) {
        disconnectLocked();
        return std::nullopt;
    }

    std::array<std::uint8_t, 7> header{};
    if (!receiveAll(socket_, header.data(), header.size())) {
        disconnectLocked();
        return std::nullopt;
    }

    const auto responseTransaction = readU16(header.data());
    const auto protocol = readU16(header.data() + 2);
    const auto length = readU16(header.data() + 4);
    if (responseTransaction != transaction || protocol != 0U || length < 2U ||
        header[6] != endpoint_.unitId || length > 260U) {
        disconnectLocked();
        return std::nullopt;
    }

    std::vector<std::uint8_t> pdu(length - 1U);
    if (!receiveAll(socket_, pdu.data(), pdu.size())) {
        disconnectLocked();
        return std::nullopt;
    }
    if (pdu.empty() || pdu[0] != function || (pdu[0] & 0x80U) != 0U) {
        return std::nullopt;
    }
    return pdu;
}

std::optional<std::uint16_t> PosixModbusTcpClient::readRegister(
    std::uint8_t function,
    std::uint16_t address
) {
    std::scoped_lock lock(mutex_);
    std::vector<std::uint8_t> payload;
    appendU16(payload, address);
    appendU16(payload, 1U);
    const auto response = transactLocked(function, payload);
    if (!response || response->size() != 4U || (*response)[1] != 2U) {
        return std::nullopt;
    }
    return readU16(response->data() + 2);
}

std::optional<std::uint16_t> PosixModbusTcpClient::readInput(
    std::uint16_t address
) {
    return readRegister(0x04U, address);
}

std::optional<std::uint16_t> PosixModbusTcpClient::readHolding(
    std::uint16_t address
) {
    return readRegister(0x03U, address);
}

std::optional<bool> PosixModbusTcpClient::readCoil(std::uint16_t address) {
    std::scoped_lock lock(mutex_);
    std::vector<std::uint8_t> payload;
    appendU16(payload, address);
    appendU16(payload, 1U);
    const auto response = transactLocked(0x01U, payload);
    if (!response || response->size() != 3U || (*response)[1] != 1U) {
        return std::nullopt;
    }
    return ((*response)[2] & 0x01U) != 0U;
}

bool PosixModbusTcpClient::writeHolding(
    std::uint16_t address,
    std::uint16_t value
) {
    std::scoped_lock lock(mutex_);
    std::vector<std::uint8_t> payload;
    appendU16(payload, address);
    appendU16(payload, value);
    const auto response = transactLocked(0x06U, payload);
    return response && response->size() == 5U &&
           readU16(response->data() + 1) == address &&
           readU16(response->data() + 3) == value;
}

}  // namespace gridex::rockpie
