#pragma once

#include "gridex/rockpie/MbusNodeTelemetry.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

namespace gridex::rockpie {

// A bounded, local NDJSON journal. It is intentionally transport-agnostic:
// a future backend recovery worker can acknowledge and drain it without the
// edge service accepting inbound connections or enabling control traffic.
struct TelemetryJournalConfig {
    std::filesystem::path path;
    std::size_t maxBytes{4U * 1024U * 1024U};
    bool enabled{true};
};

class TelemetryJournal {
  public:
    explicit TelemetryJournal(TelemetryJournalConfig config);

    [[nodiscard]] bool enabled() const noexcept;
    [[nodiscard]] const std::filesystem::path& path() const noexcept;
    [[nodiscard]] std::uint64_t nextSequence() const noexcept;

    bool appendSnapshot(std::size_t slot, const MbusNodeTelemetry& sample);
    bool appendTransition(std::size_t slot, const MbusNodeTelemetry& sample,
                          NodePollStatus previousStatus);

  private:
    TelemetryJournalConfig config_;
    std::uint64_t sequence_{0};

    [[nodiscard]] bool rotateIfNeeded(std::size_t incomingBytes) const;
    [[nodiscard]] bool append(std::string line);
    [[nodiscard]] std::string record(std::string_view kind, std::size_t slot,
                                     const MbusNodeTelemetry& sample,
                                     NodePollStatus previousStatus) const;
};

}  // namespace gridex::rockpie
