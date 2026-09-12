#ifdef ARDUINO

#include "gridex/mbus/EthernetControlServer.hpp"

#include "gridex/mbus/RegisterMap.hpp"

#include <algorithm>

namespace gridex::mbus {
namespace {

std::uint16_t readU16(const std::uint8_t* data) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(data[0]) << 8U) | data[1]
    );
}

void appendCrc(std::vector<std::uint8_t>& frame) {
    const auto crc = MbusNode::crc16(frame.data(), frame.size());
    frame.push_back(static_cast<std::uint8_t>(crc));
    frame.push_back(static_cast<std::uint8_t>(crc >> 8U));
}

std::vector<std::uint8_t> exception(
    const std::uint8_t* adu,
    std::uint8_t function,
    std::uint8_t code
) {
    return {
        adu[0], adu[1], 0, 0, 0, 3, adu[6],
        static_cast<std::uint8_t>(function | 0x80U), code,
    };
}

}  // namespace

EthernetControlServer::EthernetControlServer(
    MbusNode& node,
    EthernetControlConfig config
) : node_(node), config_(config), server_(config.port) {
    rx_.reserve(300U);
}

void EthernetControlServer::begin() {
    restart();
}

void EthernetControlServer::restart() {
    client_.stop();
    rx_.clear();
    server_.end();
    server_.begin();
    server_.setNoDelay(true);
}

void EthernetControlServer::setRockPiAddress(IPAddress address) {
    // A changed source invalidates any existing client connection.
    client_.stop();
    config_.rockPiAddress = address;
}

bool EthernetControlServer::sourceAllowed(const WiFiClient& client) const {
    return config_.rockPiAddress != IPAddress() &&
           client.remoteIP() == config_.rockPiAddress;
}

void EthernetControlServer::acceptClient() {
    if (client_ && client_.connected()) return;
    client_.stop();
    auto candidate = server_.available();
    if (!candidate) return;
    if (!sourceAllowed(candidate)) {
        ++rejectedClients_;
        candidate.stop();
        return;
    }
    client_ = candidate;
    client_.setNoDelay(true);
    rx_.clear();
}

bool EthernetControlServer::writeAllowed(
    std::uint8_t function,
    std::uint16_t start,
    std::uint16_t count
) const {
    if (function != 0x06U && function != 0x10U) return true;
    if (count == 0U) return false;
    const auto end = static_cast<std::uint32_t>(start) + count - 1U;
    return start >= reg::CommandSequence && end <= reg::CommandTtlSeconds;
}

std::vector<std::uint8_t> EthernetControlServer::handle(
    const std::uint8_t* adu,
    std::size_t size
) {
    if (size < 12U || readU16(adu + 2U) != 0U ||
        adu[6] != config_.unitId) {
        return {};
    }
    const auto function = adu[7];
    const auto start = readU16(adu + 8U);
    std::uint16_t count = 1U;
    if (function == 0x03U || function == 0x04U || function == 0x10U) {
        count = readU16(adu + 10U);
    }
    if (!writeAllowed(function, start, count)) {
        return exception(adu, function, 0x02U);
    }

    std::vector<std::uint8_t> rtu;
    rtu.reserve(size - 4U);
    rtu.push_back(node_.address());
    rtu.insert(rtu.end(), adu + 7U, adu + size);
    appendCrc(rtu);
    const auto rtuResponse = node_.processFrame(rtu.data(), rtu.size());
    if (rtuResponse.size() < 4U) return {};

    const auto pduSize = rtuResponse.size() - 3U;
    std::vector<std::uint8_t> response{
        adu[0], adu[1], 0, 0,
        static_cast<std::uint8_t>((pduSize + 1U) >> 8U),
        static_cast<std::uint8_t>((pduSize + 1U) & 0xFFU),
        config_.unitId,
    };
    response.insert(
        response.end(),
        rtuResponse.begin() + 1,
        rtuResponse.end() - 2
    );
    return response;
}

void EthernetControlServer::processFrames() {
    while (rx_.size() >= 7U) {
        const auto protocolLength = readU16(rx_.data() + 4U);
        const auto total = static_cast<std::size_t>(6U + protocolLength);
        if (protocolLength < 2U || total > 260U) {
            client_.stop();
            rx_.clear();
            return;
        }
        if (rx_.size() < total) return;
        const auto response = handle(rx_.data(), total);
        if (!response.empty()) {
            client_.write(response.data(), response.size());
            client_.flush();
        }
        rx_.erase(rx_.begin(), rx_.begin() + total);
    }
}

void EthernetControlServer::loop() {
    acceptClient();
    if (!client_ || !client_.connected()) return;
    while (client_.available() > 0 && rx_.size() < 300U) {
        const int value = client_.read();
        if (value >= 0) {
            rx_.push_back(static_cast<std::uint8_t>(value));
            lastByteMs_ = millis();
        }
    }
    processFrames();
    if (!rx_.empty() && millis() - lastByteMs_ > 1000U) {
        client_.stop();
        rx_.clear();
    }
}

bool EthernetControlServer::connected() {
    return client_ && client_.connected();
}

bool EthernetControlServer::listening() const {
    return config_.port != 0U;
}

std::uint32_t EthernetControlServer::rejectedClients() const {
    return rejectedClients_;
}

}  // namespace gridex::mbus

#endif
