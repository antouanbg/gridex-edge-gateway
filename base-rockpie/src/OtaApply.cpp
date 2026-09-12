#include <array>
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <netdb.h>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

namespace {

class Sha256 {
public:
    void update(const std::uint8_t* input, std::size_t size) {
        while (size != 0U) {
            const auto available = block_.size() - used_;
            const auto copySize = std::min(available, size);
            std::memcpy(block_.data() + used_, input, copySize);
            used_ += copySize;
            input += copySize;
            size -= copySize;
            bitCount_ += static_cast<std::uint64_t>(copySize) * 8U;
            if (used_ == block_.size()) {
                transform(block_);
                used_ = 0U;
            }
        }
    }

    [[nodiscard]] std::array<std::uint8_t, 32> finish() {
        block_[used_++] = 0x80U;
        if (used_ > 56U) {
            while (used_ < block_.size()) block_[used_++] = 0U;
            transform(block_);
            used_ = 0U;
        }
        while (used_ < 56U) block_[used_++] = 0U;
        for (std::size_t index = 0; index < 8U; ++index) {
            block_[63U - index] = static_cast<std::uint8_t>(bitCount_ >> (index * 8U));
        }
        transform(block_);
        std::array<std::uint8_t, 32> result{};
        for (std::size_t index = 0; index < state_.size(); ++index) {
            for (std::size_t byte = 0; byte < 4U; ++byte) {
                result[index * 4U + byte] = static_cast<std::uint8_t>(
                    state_[index] >> ((3U - byte) * 8U));
            }
        }
        return result;
    }

private:
    static constexpr std::array<std::uint32_t, 64> kRound{
        0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U,
        0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
        0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U,
        0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
        0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
        0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
        0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U,
        0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
        0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U,
        0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
        0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U,
        0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
        0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U,
        0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
        0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
        0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U,
    };
    std::array<std::uint32_t, 8> state_{
        0x6a09e667U, 0xbb67ae85U, 0x3c6ef372U, 0xa54ff53aU,
        0x510e527fU, 0x9b05688cU, 0x1f83d9abU, 0x5be0cd19U,
    };
    std::array<std::uint8_t, 64> block_{};
    std::size_t used_{0U};
    std::uint64_t bitCount_{0U};

    [[nodiscard]] static std::uint32_t rotateRight(std::uint32_t value, unsigned count) {
        return (value >> count) | (value << (32U - count));
    }

    void transform(const std::array<std::uint8_t, 64>& bytes) {
        std::array<std::uint32_t, 64> words{};
        for (std::size_t index = 0; index < 16U; ++index) {
            words[index] = (static_cast<std::uint32_t>(bytes[index * 4U]) << 24U) |
                           (static_cast<std::uint32_t>(bytes[index * 4U + 1U]) << 16U) |
                           (static_cast<std::uint32_t>(bytes[index * 4U + 2U]) << 8U) |
                           static_cast<std::uint32_t>(bytes[index * 4U + 3U]);
        }
        for (std::size_t index = 16U; index < words.size(); ++index) {
            const auto s0 = rotateRight(words[index - 15U], 7U) ^
                            rotateRight(words[index - 15U], 18U) ^
                            (words[index - 15U] >> 3U);
            const auto s1 = rotateRight(words[index - 2U], 17U) ^
                            rotateRight(words[index - 2U], 19U) ^
                            (words[index - 2U] >> 10U);
            words[index] = words[index - 16U] + s0 + words[index - 7U] + s1;
        }
        auto a = state_[0]; auto b = state_[1]; auto c = state_[2]; auto d = state_[3];
        auto e = state_[4]; auto f = state_[5]; auto g = state_[6]; auto h = state_[7];
        for (std::size_t index = 0; index < words.size(); ++index) {
            const auto s1 = rotateRight(e, 6U) ^ rotateRight(e, 11U) ^ rotateRight(e, 25U);
            const auto choice = (e & f) ^ (~e & g);
            const auto temp1 = h + s1 + choice + kRound[index] + words[index];
            const auto s0 = rotateRight(a, 2U) ^ rotateRight(a, 13U) ^ rotateRight(a, 22U);
            const auto majority = (a & b) ^ (a & c) ^ (b & c);
            const auto temp2 = s0 + majority;
            h = g; g = f; f = e; e = d + temp1;
            d = c; c = b; b = a; a = temp1 + temp2;
        }
        state_[0] += a; state_[1] += b; state_[2] += c; state_[3] += d;
        state_[4] += e; state_[5] += f; state_[6] += g; state_[7] += h;
    }
};

[[nodiscard]] std::string hexDigest(const std::array<std::uint8_t, 32>& digest) {
    constexpr char hex[] = "0123456789abcdef";
    std::string result;
    result.reserve(64U);
    for (const auto value : digest) {
        result.push_back(hex[value >> 4U]);
        result.push_back(hex[value & 0x0fU]);
    }
    return result;
}

[[nodiscard]] std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1U);
}

[[nodiscard]] std::optional<std::string> readToken(const std::filesystem::path& path) {
    struct stat metadata {};
    if (stat(path.c_str(), &metadata) != 0 || (metadata.st_mode & 0077) != 0) {
        return std::nullopt;
    }
    std::ifstream input(path);
    std::string token;
    std::getline(input, token);
    token = trim(token);
    if (token.length() < 32U || token.length() > 128U) return std::nullopt;
    return token;
}

[[nodiscard]] std::optional<std::string> hashFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) return std::nullopt;
    Sha256 hash;
    std::array<std::uint8_t, 8192> buffer{};
    while (input.read(reinterpret_cast<char*>(buffer.data()), buffer.size()) || input.gcount() != 0) {
        hash.update(buffer.data(), static_cast<std::size_t>(input.gcount()));
    }
    if (!input.eof()) return std::nullopt;
    return hexDigest(hash.finish());
}

[[nodiscard]] bool sendAll(int socketFd, const void* data, std::size_t size) {
    const auto* bytes = static_cast<const std::uint8_t*>(data);
    while (size != 0U) {
        const auto sent = send(socketFd, bytes, size, 0);
        if (sent <= 0) return false;
        bytes += sent;
        size -= static_cast<std::size_t>(sent);
    }
    return true;
}

[[nodiscard]] int connectIpv4(const std::string& host, std::uint16_t port) {
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo* results = nullptr;
    const auto result = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &results);
    if (result != 0 || results == nullptr) return -1;
    int socketFd = -1;
    for (auto* candidate = results; candidate != nullptr; candidate = candidate->ai_next) {
        socketFd = socket(candidate->ai_family, candidate->ai_socktype, candidate->ai_protocol);
        if (socketFd >= 0 && connect(socketFd, candidate->ai_addr, candidate->ai_addrlen) == 0) break;
        if (socketFd >= 0) close(socketFd);
        socketFd = -1;
    }
    freeaddrinfo(results);
    return socketFd;
}

struct Arguments {
    std::string host;
    std::uint16_t port{8080U};
    std::filesystem::path firmware;
    std::filesystem::path tokenFile;
    bool selfTest{false};
};

[[nodiscard]] std::optional<Arguments> parseArguments(int argc, char** argv) {
    Arguments result;
    for (int index = 1; index < argc; ++index) {
        const std::string_view argument(argv[index]);
        if (argument == "--self-test") {
            result.selfTest = true;
            continue;
        }
        if (index + 1 >= argc) return std::nullopt;
        const std::string value(argv[++index]);
        if (argument == "--host") result.host = value;
        else if (argument == "--firmware") result.firmware = value;
        else if (argument == "--token-file") result.tokenFile = value;
        else if (argument == "--port") {
            try {
                const auto parsed = std::stoul(value);
                if (parsed == 0U || parsed > 65535U) return std::nullopt;
                result.port = static_cast<std::uint16_t>(parsed);
            } catch (...) {
                return std::nullopt;
            }
        } else return std::nullopt;
    }
    if (result.selfTest) return result;
    if (result.host.empty() || result.firmware.empty() || result.tokenFile.empty()) return std::nullopt;
    return result;
}

[[nodiscard]] bool selfTest() {
    constexpr std::array<std::uint8_t, 3> input{'a', 'b', 'c'};
    Sha256 hash;
    hash.update(input.data(), input.size());
    return hexDigest(hash.finish()) ==
        "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
}

}  // namespace

int main(int argc, char** argv) {
    const auto arguments = parseArguments(argc, argv);
    if (!arguments) {
        std::cerr << "Usage: gridex_ota_apply --host <esp-host> [--port 8080] "
                     "--firmware <file> --token-file <owner-only-file>\n";
        return 64;
    }
    if (arguments->selfTest) return selfTest() ? 0 : 1;

    std::error_code error;
    const auto size = std::filesystem::file_size(arguments->firmware, error);
    if (error || size == 0U || size > 3U * 1024U * 1024U) {
        std::cerr << "Firmware file is missing or outside the accepted size limit\n";
        return 65;
    }
    const auto token = readToken(arguments->tokenFile);
    const auto digest = hashFile(arguments->firmware);
    if (!token || !digest) {
        std::cerr << "Cannot read owner-only token file or hash firmware\n";
        return 66;
    }
    const int socketFd = connectIpv4(arguments->host, arguments->port);
    if (socketFd < 0) {
        std::cerr << "Cannot connect to configured ESP32 OTA endpoint\n";
        return 67;
    }
    constexpr std::string_view boundary = "gridex-ota-v1-boundary";
    const std::string multipartStart = "--" + std::string(boundary) +
        "\r\nContent-Disposition: form-data; name=\"firmware\"; filename=\"firmware.bin\""
        "\r\nContent-Type: application/octet-stream\r\n\r\n";
    const std::string multipartEnd = "\r\n--" + std::string(boundary) + "--\r\n";
    const auto contentLength = multipartStart.size() + size + multipartEnd.size();
    std::ostringstream header;
    header << "POST /gridex/ota HTTP/1.1\r\nHost: " << arguments->host
           << "\r\nContent-Type: multipart/form-data; boundary=" << boundary
           << "\r\nContent-Length: " << contentLength
           << "\r\nX-GrideX-SHA256: " << *digest
           << "\r\nX-GrideX-Token: " << *token
           << "\r\nConnection: close\r\n\r\n";
    const auto request = header.str();
    bool success = sendAll(socketFd, request.data(), request.size()) &&
                   sendAll(socketFd, multipartStart.data(), multipartStart.size());
    std::ifstream firmware(arguments->firmware, std::ios::binary);
    std::array<char, 8192> buffer{};
    while (success && (firmware.read(buffer.data(), buffer.size()) || firmware.gcount() != 0)) {
        success = sendAll(socketFd, buffer.data(), static_cast<std::size_t>(firmware.gcount()));
    }
    if (success) success = sendAll(socketFd, multipartEnd.data(), multipartEnd.size());
    std::array<char, 256> response{};
    const auto received = recv(socketFd, response.data(), response.size() - 1U, 0);
    close(socketFd);
    const std::string_view responseView(response.data(), received > 0 ? static_cast<std::size_t>(received) : 0U);
    const auto lineEnd = responseView.find("\r\n");
    const auto statusLine = responseView.substr(0, lineEnd);
    if (!success || received <= 0 || statusLine.find(" 200 ") == std::string_view::npos) {
        std::cerr << "ESP32 OTA endpoint rejected the update: " << statusLine << '\n';
        return 68;
    }
    std::cout << "ESP32 accepted verified OTA image\n";
    return 0;
}
