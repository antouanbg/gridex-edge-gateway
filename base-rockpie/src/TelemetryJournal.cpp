#include "gridex/rockpie/TelemetryJournal.hpp"

#include <chrono>
#include <ctime>
#include <fcntl.h>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <system_error>
#include <unistd.h>

namespace gridex::rockpie {
namespace {

std::string timestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto seconds = std::chrono::system_clock::to_time_t(now);
    std::tm utc{};
    gmtime_r(&seconds, &utc);
    std::ostringstream output;
    output << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return output.str();
}

const char* pollStatusName(NodePollStatus status) {
    switch (status) {
        case NodePollStatus::Unknown: return "unknown";
        case NodePollStatus::Online: return "online";
        case NodePollStatus::TransportFailure: return "transport_failure";
        case NodePollStatus::IdentityFailure: return "identity_failure";
        case NodePollStatus::TelemetryFailure: return "telemetry_failure";
    }
    return "unknown";
}

const char* jsonBool(bool value) { return value ? "true" : "false"; }

}  // namespace

TelemetryJournal::TelemetryJournal(TelemetryJournalConfig config)
    : config_(std::move(config)) {
    if (config_.path.empty() || config_.maxBytes < 64U * 1024U) {
        config_.enabled = false;
    }
}

bool TelemetryJournal::enabled() const noexcept { return config_.enabled; }

const std::filesystem::path& TelemetryJournal::path() const noexcept {
    return config_.path;
}

std::uint64_t TelemetryJournal::nextSequence() const noexcept {
    return sequence_ + 1U;
}

bool TelemetryJournal::appendSnapshot(std::size_t slot,
                                      const MbusNodeTelemetry& sample) {
    return append(record("snapshot", slot, sample, NodePollStatus::Unknown));
}

bool TelemetryJournal::appendTransition(std::size_t slot,
                                        const MbusNodeTelemetry& sample,
                                        NodePollStatus previousStatus) {
    return append(record("transition", slot, sample, previousStatus));
}

bool TelemetryJournal::rotateIfNeeded(std::size_t incomingBytes) const {
    std::error_code error;
    std::filesystem::create_directories(config_.path.parent_path(), error);
    if (error) return false;
    const auto current = std::filesystem::file_size(config_.path, error);
    if (error && error != std::errc::no_such_file_or_directory) return false;
    if (!error && current + incomingBytes <= config_.maxBytes) return true;
    if (error) return true;

    const auto prior = config_.path.string() + ".1";
    std::filesystem::remove(prior, error);
    error.clear();
    std::filesystem::rename(config_.path, prior, error);
    return !error;
}

bool TelemetryJournal::append(std::string line) {
    if (!enabled()) return true;
    line.push_back('\n');
    if (!rotateIfNeeded(line.size())) return false;

    const int descriptor = open(config_.path.c_str(), O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0640);
    if (descriptor < 0) return false;
    const auto expected = static_cast<ssize_t>(line.size());
    const auto written = write(descriptor, line.data(), line.size());
    const bool synced = written == expected && fsync(descriptor) == 0;
    (void)close(descriptor);
    if (!synced) return false;
    ++sequence_;
    return true;
}

std::string TelemetryJournal::record(std::string_view kind, std::size_t slot,
                                     const MbusNodeTelemetry& sample,
                                     NodePollStatus previousStatus) const {
    std::ostringstream output;
    output << "{\"schemaVersion\":1,\"sequence\":" << nextSequence()
           << ",\"observedAt\":\"" << timestamp()
           << "\",\"kind\":\"" << kind
           << "\",\"slot\":" << slot
           << ",\"online\":" << jsonBool(sample.online)
           << ",\"pollStatus\":\"" << pollStatusName(sample.pollStatus)
           << "\",\"previousPollStatus\":\"" << pollStatusName(previousStatus)
           << "\",\"consecutiveFailures\":" << sample.consecutiveFailures
           << ",\"heartbeat\":" << sample.heartbeat
           << ",\"actualPowerKw\":" << sample.actualPowerKw
           << ",\"energyWh\":" << sample.energyWh
           << ",\"quality\":" << sample.quality
           << ",\"alarmBits\":" << sample.alarmBits
           << ",\"lastError\":" << sample.lastError << "}";
    return output.str();
}

}  // namespace gridex::rockpie
