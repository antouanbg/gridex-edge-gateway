#include "gridex/NorthboundMap.hpp"
#include "gridex/rockpie/NorthboundModbusTcpServer.hpp"
#include "gridex/rockpie/NorthboundRegisterBank.hpp"

#include <arpa/inet.h>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <span>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace {

std::vector<std::uint8_t> roundTrip(
    std::uint16_t port,
    std::span<const std::uint8_t> request,
    std::size_t responseSize
) {
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    assert(fd >= 0);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    assert(::inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) == 1);
    assert(::connect(
        fd,
        reinterpret_cast<const sockaddr*>(&address),
        sizeof(address)
    ) == 0);
    assert(::send(fd, request.data(), request.size(), 0) ==
           static_cast<ssize_t>(request.size()));
    std::vector<std::uint8_t> response(responseSize);
    assert(::recv(fd, response.data(), response.size(), MSG_WAITALL) ==
           static_cast<ssize_t>(response.size()));
    ::close(fd);
    return response;
}

}  // namespace

int main() {
    gridex::rockpie::NorthboundRegisterBank bank;
    assert(!bank.takeCommand());
    gridex::ControllerSnapshot snapshot;
    snapshot.state = gridex::EdgeState::Ready;
    snapshot.emsConnected = true;
    snapshot.heartbeatOk = true;
    snapshot.battery = {
        .actualPowerKw = -25.4,
        .socPct = 72.5,
        .sohPct = 98.0,
        .voltageV = 760.0,
        .currentA = -12.3,
        .maxChargeKw = 100.0,
        .maxDischargeKw = 120.0,
        .statusCode = 2,
        .bmsSystemFlags = 0,
        .quality = gridex::Quality::Good,
        .limitsValid = true,
        .pcsPowerOn = true,
        .pcsGridTied = true,
        .pcsCurrentSourceMode = true,
        .pcsFault = false,
        .pcsCommunicationFault = false,
        .bmsFault = false,
        .bmsCommunicationFault = false,
        .controlReady = true,
    };
    snapshot.command.appliedPowerKw = -25.4;

    bank.publish(
        snapshot,
        {.maxChargeKw = 80.0, .maxDischargeKw = 90.0}
    );
    const auto values = bank.readInput(
        0,
        gridex::northbound::InputRegisterCount
    );
    assert(values);
    assert(static_cast<std::int16_t>(
        (*values)[gridex::northbound::input::ActualPowerKwX10]
    ) == -254);
    assert((*values)[gridex::northbound::input::SocPctX10] == 725U);
    assert((*values)[gridex::northbound::input::ControlReady] == 1U);
    assert(
        (*values)[gridex::northbound::input::ConfiguredMaxChargeKwX10] ==
        800U
    );

    const std::array<std::uint16_t, 4> commandRegisters{
        1U,
        static_cast<std::uint16_t>(static_cast<std::int16_t>(-300)),
        1U,
        10U,
    };
    assert(bank.writeHolding(0, commandRegisters));
    const auto command = bank.takeCommand();
    assert(command);
    assert(command->enabled);
    assert(command->requestedPowerKw == -30.0);
    assert(!bank.takeCommand());

    const std::array<std::uint16_t, 1> heartbeat{11U};
    assert(bank.writeHolding(
        gridex::northbound::holding::EmsHeartbeat,
        heartbeat
    ));
    const auto refreshed = bank.takeCommand();
    assert(refreshed);
    assert(refreshed->heartbeat == 11U);
    assert(refreshed->requestedPowerKw == -30.0);

    const std::array<std::uint16_t, 1> disabled{0U};
    const std::array<std::uint16_t, 1> nextSequence{2U};
    assert(bank.writeHolding(
        gridex::northbound::holding::CommandEnable,
        disabled
    ));
    assert(bank.writeHolding(
        gridex::northbound::holding::CommandSequence,
        nextSequence
    ));
    const auto stopped = bank.takeCommand();
    assert(stopped);
    assert(!stopped->enabled);
    assert(stopped->requestedPowerKw == 0.0);

    constexpr std::uint16_t testPort = 21502;
    gridex::rockpie::NorthboundModbusTcpServer server(
        bank,
        {.bindAddress = "127.0.0.1", .port = testPort, .unitId = 1}
    );
    assert(server.start());
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    const std::array<std::uint8_t, 12> readSocRequest{
        0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x01,
        0x04, 0x00, 0x01, 0x00, 0x01,
    };
    const auto readSocResponse = roundTrip(testPort, readSocRequest, 11U);
    assert(readSocResponse[7] == 0x04U);
    assert(readSocResponse[8] == 0x02U);
    assert(readSocResponse[9] == 0x02U);
    assert(readSocResponse[10] == 0xD5U);

    const std::array<std::uint8_t, 12> heartbeatRequest{
        0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x01,
        0x06, 0x00, 0x03, 0x00, 0x0C,
    };
    const auto heartbeatResponse = roundTrip(
        testPort,
        heartbeatRequest,
        heartbeatRequest.size()
    );
    assert(heartbeatResponse == std::vector<std::uint8_t>(
        heartbeatRequest.begin(),
        heartbeatRequest.end()
    ));
    server.stop();

    std::cout << "All GrideX northbound register tests passed.\n";
    return 0;
}
