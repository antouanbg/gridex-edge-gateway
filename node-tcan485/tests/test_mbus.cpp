#include "gridex/mbus/MbusNode.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

namespace {

void appendU16(std::vector<std::uint8_t>& bytes, std::uint16_t value) {
    bytes.push_back(static_cast<std::uint8_t>(value >> 8U));
    bytes.push_back(static_cast<std::uint8_t>(value));
}

std::vector<std::uint8_t> request(std::vector<std::uint8_t> pdu) {
    const auto crc = gridex::mbus::MbusNode::crc16(pdu.data(), pdu.size());
    pdu.push_back(static_cast<std::uint8_t>(crc));
    pdu.push_back(static_cast<std::uint8_t>(crc >> 8U));
    return pdu;
}

void testIdentityRead() {
    gridex::mbus::MbusNode node(
        {7, gridex::mbus::NodeType::Meter, 0x1122334455667788ULL}
    );
    auto frame = request({7, 0x03, 0x00, 0x00, 0x00, 0x0A});
    const auto response = node.processFrame(frame.data(), frame.size());
    assert(response.size() == 25U);
    assert(response[0] == 7U);
    assert(response[1] == 0x03U);
    assert(response[2] == 20U);
    assert(response[3] == 0x47U && response[4] == 0x58U);
    assert(response[7] == 0x00U && response[8] == 0x04U);
}

void testConfigurationWrite() {
    gridex::mbus::MbusNode node(
        {205, gridex::mbus::NodeType::Unconfigured, 42}
    );
    std::vector<std::uint8_t> body{205, 0x10};
    appendU16(body, gridex::mbus::reg::RequestedAddress);
    appendU16(body, 3);
    body.push_back(6);
    appendU16(body, 12);
    appendU16(body, static_cast<std::uint16_t>(gridex::mbus::NodeType::Evse));
    appendU16(body, gridex::mbus::reg::ApplyKey);
    auto frame = request(std::move(body));
    const auto response = node.processFrame(frame.data(), frame.size());
    assert(!response.empty());
    assert(node.address() == 12U);
    assert(node.type() == gridex::mbus::NodeType::Evse);
    assert(node.takeConfigurationChanged());
    assert(!node.takeConfigurationChanged());
}

void testCrcAndAddressRejection() {
    gridex::mbus::MbusNode node(
        {9, gridex::mbus::NodeType::Unconfigured, 1}
    );
    auto wrongAddress = request({8, 0x03, 0x00, 0x00, 0x00, 0x01});
    assert(node.processFrame(wrongAddress.data(), wrongAddress.size()).empty());
    wrongAddress.back() ^= 0xFFU;
    assert(node.processFrame(wrongAddress.data(), wrongAddress.size()).empty());
}

}  // namespace

int main() {
    testIdentityRead();
    testConfigurationWrite();
    testCrcAndAddressRejection();
    std::cout << "All GrideX MBUS node tests passed.\n";
    return 0;
}
