#include "gridex/rockpie/NodeTcpPollingService.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <thread>
#include <vector>

namespace {

bool receiveAll(int socket, std::uint8_t* buffer, std::size_t count) {
    std::size_t received = 0;
    while (received < count) {
        const auto result = recv(socket, buffer + received, count - received, 0);
        if (result <= 0) return false;
        received += static_cast<std::size_t>(result);
    }
    return true;
}

bool sendAll(int socket, const std::vector<std::uint8_t>& bytes) {
    std::size_t sent = 0;
    while (sent < bytes.size()) {
        const auto result = send(socket, bytes.data() + sent, bytes.size() - sent, 0);
        if (result <= 0) return false;
        sent += static_cast<std::size_t>(result);
    }
    return true;
}

std::uint16_t readU16(const std::uint8_t* bytes) {
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(bytes[0]) << 8U) | bytes[1]);
}

void appendU16(std::vector<std::uint8_t>& bytes, std::uint16_t value) {
    bytes.push_back(static_cast<std::uint8_t>(value >> 8U));
    bytes.push_back(static_cast<std::uint8_t>(value));
}

}  // namespace

int main() {
    const int listener = socket(AF_INET, SOCK_STREAM, 0);
    assert(listener >= 0);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    assert(bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0);
    assert(listen(listener, 1) == 0);
    socklen_t addressSize = sizeof(address);
    assert(getsockname(listener, reinterpret_cast<sockaddr*>(&address), &addressSize) == 0);

    std::thread server([&] {
        const int peer = accept(listener, nullptr, nullptr);
        assert(peer >= 0);
        std::array<std::uint8_t, 12> request{};
        while (receiveAll(peer, request.data(), request.size())) {
            assert(request[7] == 0x03U);
            const auto start = readU16(request.data() + 8);
            const auto count = readU16(request.data() + 10);
            std::vector<std::uint16_t> registers(count, 0U);
            if (start == 0U && count == 13U) {
                registers[0] = 0x4758U;
                registers[2] = 4U;
                registers[3] = 2U;
                registers[5] = 77U;
                registers[6] = 1U;
                registers[12] = 9U;
            } else if (start == 0x40U && count == 6U) {
                registers = {static_cast<std::uint16_t>(-125), 0U, 99U, 3U, 5U, 0U};
            } else if (start == 0x46U && count == 9U) {
                registers = {2U, 1U, 0U, 1U, 1U, 3U, 2U, 1U, 0U};
            } else {
                assert(false);
            }
            std::vector<std::uint8_t> response{
                request[0], request[1], 0U, 0U, 0U,
                static_cast<std::uint8_t>((count * 2U) + 3U), 1U,
                0x03U, static_cast<std::uint8_t>(count * 2U),
            };
            for (const auto value : registers) appendU16(response, value);
            assert(sendAll(peer, response));
        }
        close(peer);
    });

    gridex::rockpie::NodeTcpPollingService polling({
        .endpoints = {{.host = "127.0.0.1", .port = ntohs(address.sin_port), .unitId = 1}},
        .interval = std::chrono::milliseconds(100),
        .timeout = std::chrono::milliseconds(500),
    });
    polling.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    const auto samples = polling.samples();
    assert(samples.size() == 1U);
    assert(samples[0].online);
    assert(samples[0].driverId == 77U);
    assert(samples[0].actualPowerKw == -12.5);
    assert(samples[0].energyWh == 99U);
    assert(samples[0].deviceState == 3U);
    assert(samples[0].alarmBits == 5U);
    assert(samples[0].pollStatus == gridex::rockpie::NodePollStatus::Online);
    assert(samples[0].consecutiveFailures == 0U);
    assert(samples[0].ethernetStatus == 2U);
    assert(samples[0].modbusTcpStatus == 1U);
    assert(samples[0].recoveryCount == 3U);
    polling.stop();
    server.join();
    close(listener);
}
