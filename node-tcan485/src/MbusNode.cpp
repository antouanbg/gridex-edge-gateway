#include "gridex/mbus/MbusNode.hpp"

#include <algorithm>

namespace gridex::mbus {
namespace {

std::uint16_t readU16(const std::uint8_t* bytes) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(bytes[0]) << 8U) | bytes[1]
    );
}

void appendU16(std::vector<std::uint8_t>& bytes, std::uint16_t value) {
    bytes.push_back(static_cast<std::uint8_t>(value >> 8U));
    bytes.push_back(static_cast<std::uint8_t>(value & 0xFFU));
}

bool validType(std::uint16_t type) {
    return type == static_cast<std::uint16_t>(NodeType::Meter) ||
           type == static_cast<std::uint16_t>(NodeType::Evse) ||
           type == static_cast<std::uint16_t>(NodeType::SecondCabinet);
}

}  // namespace

MbusNode::MbusNode(NodeConfig config) : config_(config) {
    if (config_.address == 0U) {
        config_.address = temporaryAddress(config_.uid);
    }
    initializeIdentity();
}

std::uint8_t MbusNode::temporaryAddress(std::uint64_t uid) {
    std::uint32_t folded = static_cast<std::uint32_t>(uid) ^
                           static_cast<std::uint32_t>(uid >> 32U);
    folded ^= folded >> 16U;
    return static_cast<std::uint8_t>(200U + (folded % 48U));
}

void MbusNode::initializeIdentity() {
    registers_[reg::Magic] = reg::MagicValue;
    registers_[reg::MapVersion] = reg::MapVersionValue;
    registers_[reg::NodeType] = static_cast<std::uint16_t>(config_.type);
    registers_[reg::NodeState] = static_cast<std::uint16_t>(
        config_.type == NodeType::Unconfigured
            ? NodeState::AwaitingAddress
            : NodeState::DriverMissing
    );
    registers_[reg::NodeAddress] = config_.address;
    registers_[reg::Uid0] = static_cast<std::uint16_t>(config_.uid >> 48U);
    registers_[reg::Uid1] = static_cast<std::uint16_t>(config_.uid >> 32U);
    registers_[reg::Uid2] = static_cast<std::uint16_t>(config_.uid >> 16U);
    registers_[reg::Uid3] = static_cast<std::uint16_t>(config_.uid);
    registers_[reg::RequestedAddress] = config_.address;
    registers_[reg::RequestedNodeType] =
        static_cast<std::uint16_t>(config_.type);
}

std::uint16_t MbusNode::crc16(const std::uint8_t* bytes, std::size_t size) {
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

std::vector<std::uint8_t> MbusNode::withCrc(
    std::vector<std::uint8_t> response
) {
    const auto crc = crc16(response.data(), response.size());
    response.push_back(static_cast<std::uint8_t>(crc & 0xFFU));
    response.push_back(static_cast<std::uint8_t>(crc >> 8U));
    return response;
}

std::vector<std::uint8_t> MbusNode::exception(
    std::uint8_t requestAddress,
    std::uint8_t function,
    std::uint8_t code
) const {
    return withCrc({requestAddress, static_cast<std::uint8_t>(function | 0x80U), code});
}

void MbusNode::applyPendingConfiguration() {
    if (registers_[reg::ApplyConfiguration] != reg::ApplyKey) {
        return;
    }
    registers_[reg::ApplyConfiguration] = 0U;
    const auto requestedAddress = registers_[reg::RequestedAddress];
    const auto requestedType = registers_[reg::RequestedNodeType];
    if (requestedAddress < 1U || requestedAddress > 247U ||
        !validType(requestedType)) {
        return;
    }
    config_.address = static_cast<std::uint8_t>(requestedAddress);
    config_.type = static_cast<NodeType>(requestedType);
    registers_[reg::NodeAddress] = config_.address;
    registers_[reg::NodeType] = requestedType;
    registers_[reg::NodeState] =
        static_cast<std::uint16_t>(NodeState::DriverMissing);
    configurationChanged_ = true;
}

std::vector<std::uint8_t> MbusNode::processFrame(
    const std::uint8_t* frame,
    std::size_t size
) {
    if (size < 4U) {
        return {};
    }
    const auto expectedCrc = crc16(frame, size - 2U);
    const auto receivedCrc = static_cast<std::uint16_t>(
        frame[size - 2U] | (static_cast<std::uint16_t>(frame[size - 1U]) << 8U)
    );
    if (expectedCrc != receivedCrc) {
        return {};
    }

    const auto requestAddress = frame[0];
    const bool broadcast = requestAddress == 0U;
    if (!broadcast && requestAddress != config_.address) {
        return {};
    }
    const auto function = frame[1];

    if ((function == 0x03U || function == 0x04U) && size == 8U) {
        if (broadcast) {
            return {};
        }
        const auto start = readU16(frame + 2);
        const auto count = readU16(frame + 4);
        if (count == 0U || count > 64U ||
            static_cast<std::size_t>(start) + count > registers_.size()) {
            return exception(requestAddress, function, 0x02U);
        }
        std::vector<std::uint8_t> response{
            requestAddress,
            function,
            static_cast<std::uint8_t>(count * 2U),
        };
        for (std::uint16_t i = 0; i < count; ++i) {
            appendU16(response, registers_[start + i]);
        }
        return withCrc(std::move(response));
    }

    if (function == 0x06U && size == 8U) {
        const auto address = readU16(frame + 2);
        const auto value = readU16(frame + 4);
        if (address >= registers_.size()) {
            return broadcast ? std::vector<std::uint8_t>{}
                             : exception(requestAddress, function, 0x02U);
        }
        registers_[address] = value;
        applyPendingConfiguration();
        if (broadcast) {
            return {};
        }
        return std::vector<std::uint8_t>(frame, frame + size);
    }

    if (function == 0x10U && size >= 9U) {
        const auto start = readU16(frame + 2);
        const auto count = readU16(frame + 4);
        const auto byteCount = frame[6];
        if (count == 0U || byteCount != count * 2U || size != 9U + byteCount ||
            static_cast<std::size_t>(start) + count > registers_.size()) {
            return broadcast ? std::vector<std::uint8_t>{}
                             : exception(requestAddress, function, 0x03U);
        }
        for (std::uint16_t i = 0; i < count; ++i) {
            registers_[start + i] = readU16(frame + 7U + i * 2U);
        }
        applyPendingConfiguration();
        if (broadcast) {
            return {};
        }
        std::vector<std::uint8_t> response{requestAddress, function};
        appendU16(response, start);
        appendU16(response, count);
        return withCrc(std::move(response));
    }

    return broadcast ? std::vector<std::uint8_t>{}
                     : exception(requestAddress, function, 0x01U);
}

std::uint8_t MbusNode::address() const {
    return config_.address;
}

NodeType MbusNode::type() const {
    return config_.type;
}

std::uint64_t MbusNode::uid() const {
    return config_.uid;
}

bool MbusNode::takeConfigurationChanged() {
    const bool value = configurationChanged_;
    configurationChanged_ = false;
    return value;
}

void MbusNode::setRegister(std::uint16_t address, std::uint16_t value) {
    if (address < registers_.size()) {
        registers_[address] = value;
    }
}

std::uint16_t MbusNode::registerValue(std::uint16_t address) const {
    return address < registers_.size() ? registers_[address] : 0U;
}

}  // namespace gridex::mbus
