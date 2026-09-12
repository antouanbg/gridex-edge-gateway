#include "gridex/rockpie/PosixModbusTcpClient.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <array>
#include <cassert>
#include <thread>
#include <vector>

// Loopback simulator only: verifies the actual FC04 request/response boundary.
void checkResponse(const std::vector<unsigned char>& pdu, bool expected) {
    const int listener = socket(AF_INET, SOCK_STREAM, 0);
    assert(listener >= 0);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    assert(bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0);
    assert(listen(listener, 1) == 0);
    socklen_t size = sizeof(address);
    assert(getsockname(listener, reinterpret_cast<sockaddr*>(&address), &size) == 0);
    std::thread server([&] {
        const int peer = accept(listener, nullptr, nullptr);
        assert(peer >= 0);
        std::array<unsigned char, 12> request{};
        std::size_t offset = 0;
        while (offset < request.size()) {
            const auto received = recv(peer, request.data() + offset, request.size() - offset, 0);
            assert(received > 0);
            offset += static_cast<std::size_t>(received);
        }
        assert(request[2] == 0 && request[3] == 0);
        assert(request[4] == 0 && request[5] == 6 && request[6] == 1);
        assert(request[7] == 4 && request[8] == 0 && request[9] == 122);
        assert(request[10] == 0 && request[11] == 4);
        std::vector<unsigned char> response{
            request[0], request[1], 0, 0, 0,
            static_cast<unsigned char>(pdu.size() + 1), 1
        };
        response.insert(response.end(), pdu.begin(), pdu.end());
        // Fragment the reply to exercise receiveAll rather than packet assumptions.
        for (const auto byte : response) assert(send(peer, &byte, 1, 0) == 1);
        close(peer);
    });
    gridex::rockpie::PosixModbusTcpClient client({
        .host = "127.0.0.1",
        .port = ntohs(address.sin_port),
        .unitId = 1,
        .timeout = std::chrono::milliseconds(1000)
    });
    const auto result = client.readInputRange(122, 4);
    assert(result.has_value() == expected);
    if (expected) {
        assert(*result == std::vector<std::uint16_t>({0, 12345, 1, 33229}));
    }
    server.join();
    close(listener);
}

int main() {
    checkResponse({4, 8, 0, 0, 0x30, 0x39, 0, 1, 0x81, 0xCD}, true);
    checkResponse({4, 6, 0, 0, 0x30, 0x39, 0, 1}, false);
    checkResponse({0x84, 2}, false);
    checkResponse({3, 8, 0, 0, 0x30, 0x39, 0, 1, 0x81, 0xCD}, false);
    gridex::rockpie::PosixModbusTcpClient client({.host = "127.0.0.1", .port = 0});
    assert(!client.readInputRange(122, 0));
    assert(!client.readInputRange(122, 126));
    assert(!client.readInputRange(65535, 2));
}
