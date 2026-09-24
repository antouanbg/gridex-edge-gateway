#pragma once
#include <fstream>
#include <istream>
#include <optional>
#include <string>

namespace gridex::rockpie {
// Linux thermal sysfs values are integer milli-degrees Celsius, not BESS temperature.
inline std::optional<double> parseCpuTemperature(std::istream& input) {
    long milli{};
    if (!(input >> milli) || milli < -40000 || milli > 150000) return std::nullopt;
    std::string extra;
    if (input >> extra) return std::nullopt;
    return static_cast<double>(milli) / 1000.0;
}
inline std::optional<double> readCpuTemperature(const std::string& path) {
    std::ifstream input(path);
    return parseCpuTemperature(input);
}
} // namespace gridex::rockpie
