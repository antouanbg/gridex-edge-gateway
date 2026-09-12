#include "gridex/rockpie/TelemetryJournal.hpp"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

int main() {
    const auto directory = std::filesystem::temp_directory_path() /
        ("gridex-journal-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto journalPath = directory / "telemetry.ndjson";
    gridex::rockpie::TelemetryJournal journal({
        .path = journalPath,
        .maxBytes = 64U * 1024U,
        .enabled = true,
    });
    gridex::rockpie::MbusNodeTelemetry sample{
        .address = 7U,
        .heartbeat = 42U,
        .actualPowerKw = -12.5,
        .energyWh = 1234U,
        .online = true,
        .pollStatus = gridex::rockpie::NodePollStatus::Online,
    };
    assert(journal.appendTransition(1U, sample,
                                    gridex::rockpie::NodePollStatus::TransportFailure));
    assert(journal.appendSnapshot(1U, sample));
    assert(journal.nextSequence() == 3U);

    std::ifstream input(journalPath);
    std::string first;
    std::getline(input, first);
    assert(first.find("\"kind\":\"transition\"") != std::string::npos);
    assert(first.find("\"pollStatus\":\"online\"") != std::string::npos);
    assert(first.find("\"previousPollStatus\":\"transport_failure\"") != std::string::npos);

    std::filesystem::remove_all(directory);
}
