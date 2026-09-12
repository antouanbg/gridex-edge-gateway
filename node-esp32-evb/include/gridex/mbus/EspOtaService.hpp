#pragma once

#ifdef ARDUINO

#include <Arduino.h>
#include <WebServer.h>

namespace gridex::mbus {

struct EspOtaConfig {
    std::uint16_t port{8080};
    IPAddress rockPiAddress{};
    // SHA-256 of the OTA secret. The ESP32 retains no reusable OTA secret.
    String tokenHash{};
};

// Local OT OTA endpoint. It never accepts a direct VPN or Internet caller:
// the TCP peer must be the configured ROCK Pi and must present the locally
// provisioned secret. Firmware integrity is checked with a SHA-256 header.
class EspOtaService {
public:
    explicit EspOtaService(EspOtaConfig config);

    void begin();
    void loop();
    void setConfig(EspOtaConfig config);
    [[nodiscard]] bool enabled() const;
    [[nodiscard]] std::uint32_t rejectedRequests() const;
    [[nodiscard]] static String sha256Hex(const String& value);

private:
    EspOtaConfig config_;
    WebServer server_;
    bool started_{false};
    bool accepted_{false};
    bool completed_{false};
    bool restartPending_{false};
    unsigned long restartAtMs_{0};
    std::uint32_t rejectedRequests_{0};
    std::uint32_t writtenBytes_{0};
    String expectedSha256_;
    const char* failureReason_{"not started"};

    void configureRoutes();
    void handleUpload();
    void handleResult();
    [[nodiscard]] bool sourceAllowed();
    [[nodiscard]] bool requestAuthorized();
    [[nodiscard]] bool validSha256(const String& value) const;
    [[nodiscard]] bool secureEquals(const String& left, const String& right) const;
    void reject(std::uint16_t status, const char* message);
    void fail(const char* reason);
};

}  // namespace gridex::mbus

#endif
