#include "gridex/rockpie/SystemTelemetry.hpp"
#include "gridex/rockpie/CpuTemperature.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <utility>

namespace gridex::rockpie {
namespace {
void add(std::vector<SystemTelemetrySample>& out, std::string id, std::string unit, double value) {
    if (std::isfinite(value) && value >= 0.0) out.push_back({std::move(id), std::move(unit), value});
}
}

std::vector<SystemTelemetrySample> readSystemTelemetry(const std::string& dataDirectory,
                                                       const std::string& journalPath,
                                                       const std::string& cpuTemperaturePath,
                                                       bool includeCpuTemperature) {
    std::vector<SystemTelemetrySample> out;
    if (includeCpuTemperature) {
        if (const auto temperature = readCpuTemperature(cpuTemperaturePath)) out.push_back({"cpuTemperatureC", "Cel", *temperature});
    }
    std::ifstream uptime("/proc/uptime");
    double seconds = 0.0;
    if (uptime >> seconds) add(out, "uptimeSeconds", "s", seconds);
    std::ifstream load("/proc/loadavg");
    double load1 = 0.0;
    if (load >> load1) add(out, "load1", "load", load1);
    std::ifstream memory("/proc/meminfo");
    std::string key, unit;
    std::uint64_t value = 0;
    while (memory >> key >> value >> unit) {
        if (key == "MemAvailable:") { add(out, "memoryAvailableBytes", "bytes", static_cast<double>(value) * 1024.0); break; }
    }
    std::error_code error;
    const auto space = std::filesystem::space(dataDirectory, error);
    if (!error) add(out, "storageDataFreeBytes", "bytes", static_cast<double>(space.available));
    const auto journalSize = std::filesystem::file_size(journalPath, error);
    if (!error) add(out, "journalSizeBytes", "bytes", static_cast<double>(journalSize));
    return out;
}
} // namespace gridex::rockpie
