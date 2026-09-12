#ifdef ARDUINO

#include "gridex/mbus/EspOtaService.hpp"

#include <Update.h>
#include <mbedtls/sha256.h>

namespace gridex::mbus {
namespace {

constexpr const char* kPath = "/gridex/ota";
constexpr const char* kShaHeader = "X-GrideX-SHA256";
constexpr const char* kTokenHeader = "X-GrideX-Token";
constexpr std::uint32_t kMaxFirmwareBytes = 3U * 1024U * 1024U;

mbedtls_sha256_context hashContext;
bool hashStarted = false;

String finishFirmwareHash() {
    std::uint8_t digest[32]{};
    if (!hashStarted || mbedtls_sha256_finish_ret(&hashContext, digest) != 0) {
        mbedtls_sha256_free(&hashContext);
        hashStarted = false;
        return {};
    }
    mbedtls_sha256_free(&hashContext);
    hashStarted = false;
    String result;
    result.reserve(64U);
    constexpr char kHex[] = "0123456789abcdef";
    for (const auto value : digest) {
        result += kHex[value >> 4U];
        result += kHex[value & 0x0FU];
    }
    return result;
}

}  // namespace

EspOtaService::EspOtaService(EspOtaConfig config)
    : config_(std::move(config)), server_(config_.port) {}

void EspOtaService::configureRoutes() {
    const char* headers[] = {kShaHeader, kTokenHeader};
    server_.collectHeaders(headers, 2U);
    server_.on(kPath, HTTP_POST,
        [this] { handleResult(); },
        [this] { handleUpload(); }
    );
    server_.onNotFound([this] { reject(404U, "not found"); });
}

void EspOtaService::begin() {
    if (started_ || !enabled()) return;
    configureRoutes();
    server_.begin();
    started_ = true;
}

void EspOtaService::setConfig(EspOtaConfig config) {
    // Changing the source or secret is intentionally a local provisioning
    // action. Reboot is required so the listener cannot keep stale state.
    config_ = std::move(config);
    ESP.restart();
}

bool EspOtaService::enabled() const {
    return config_.rockPiAddress != IPAddress() &&
           validSha256(config_.tokenHash);
}

std::uint32_t EspOtaService::rejectedRequests() const {
    return rejectedRequests_;
}

bool EspOtaService::sourceAllowed() {
    return enabled() && server_.client().remoteIP() == config_.rockPiAddress;
}

bool EspOtaService::secureEquals(const String& left, const String& right) const {
    if (left.length() != right.length()) return false;
    std::uint8_t difference = 0U;
    for (std::size_t index = 0; index < left.length(); ++index) {
        difference |= static_cast<std::uint8_t>(left[index] ^ right[index]);
    }
    return difference == 0U;
}

bool EspOtaService::validSha256(const String& value) const {
    if (value.length() != 64U) return false;
    for (const auto character : value) {
        const bool digit = character >= '0' && character <= '9';
        const bool lower = character >= 'a' && character <= 'f';
        const bool upper = character >= 'A' && character <= 'F';
        if (!digit && !lower && !upper) return false;
    }
    return true;
}

bool EspOtaService::requestAuthorized() {
    return sourceAllowed() &&
           validSha256(server_.header(kShaHeader)) &&
           secureEquals(sha256Hex(server_.header(kTokenHeader)), config_.tokenHash);
}

String EspOtaService::sha256Hex(const String& value) {
    mbedtls_sha256_context context;
    mbedtls_sha256_init(&context);
    std::uint8_t digest[32]{};
    const bool complete = mbedtls_sha256_starts_ret(&context, 0) == 0 &&
        mbedtls_sha256_update_ret(
            &context,
            reinterpret_cast<const std::uint8_t*>(value.c_str()),
            value.length()
        ) == 0 &&
        mbedtls_sha256_finish_ret(&context, digest) == 0;
    mbedtls_sha256_free(&context);
    if (!complete) return {};
    String result;
    result.reserve(64U);
    constexpr char kHex[] = "0123456789abcdef";
    for (const auto byte : digest) {
        result += kHex[byte >> 4U];
        result += kHex[byte & 0x0FU];
    }
    return result;
}

void EspOtaService::reject(std::uint16_t status, const char* message) {
    ++rejectedRequests_;
    server_.send(status, "text/plain", message);
}

void EspOtaService::fail(const char* reason) {
    failureReason_ = reason;
    Serial.printf("GrideX: OTA rejected (%s)\n", reason);
}

void EspOtaService::handleUpload() {
    const auto& upload = server_.upload();
    if (upload.status == UPLOAD_FILE_START) {
        accepted_ = requestAuthorized();
        completed_ = false;
        failureReason_ = "upload incomplete";
        writtenBytes_ = 0U;
        expectedSha256_ = server_.header(kShaHeader);
        if (!accepted_) {
            fail("authorization");
            ++rejectedRequests_;
            return;
        }
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            accepted_ = false;
            fail("flash initialization");
            ++rejectedRequests_;
            return;
        }
        mbedtls_sha256_init(&hashContext);
        if (mbedtls_sha256_starts_ret(&hashContext, 0) != 0) {
            Update.abort();
            accepted_ = false;
            fail("hash initialization");
            ++rejectedRequests_;
            return;
        }
        hashStarted = true;
        return;
    }

    if (!accepted_) return;
    if (upload.status == UPLOAD_FILE_WRITE) {
        if (upload.currentSize == 0U ||
            writtenBytes_ + upload.currentSize > kMaxFirmwareBytes ||
            Update.write(const_cast<std::uint8_t*>(upload.buf), upload.currentSize) != upload.currentSize ||
            mbedtls_sha256_update_ret(&hashContext, upload.buf, upload.currentSize) != 0) {
            Update.abort();
            accepted_ = false;
            fail("chunk write or size");
            ++rejectedRequests_;
            return;
        }
        writtenBytes_ += upload.currentSize;
        return;
    }

    if (upload.status == UPLOAD_FILE_END) {
        const auto digest = finishFirmwareHash();
        if (writtenBytes_ == 0U ||
            !secureEquals(digest, expectedSha256_) ||
            !Update.end(true)) {
            Update.abort();
            accepted_ = false;
            fail("integrity or flash finalization");
            ++rejectedRequests_;
            return;
        }
        completed_ = true;
        return;
    }

    if (upload.status == UPLOAD_FILE_ABORTED) {
        Update.abort();
        accepted_ = false;
        fail("upload aborted");
        ++rejectedRequests_;
    }
}

void EspOtaService::handleResult() {
    if (!completed_) {
        server_.send(422U, "text/plain", failureReason_);
        return;
    }
    server_.send(200U, "text/plain", "OTA accepted; restarting");
    restartPending_ = true;
    restartAtMs_ = millis() + 250U;
}

void EspOtaService::loop() {
    if (started_) server_.handleClient();
    if (restartPending_ && static_cast<long>(millis() - restartAtMs_) >= 0L) {
        ESP.restart();
    }
}

}  // namespace gridex::mbus

#endif
