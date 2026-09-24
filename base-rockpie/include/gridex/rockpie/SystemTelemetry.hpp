#pragma once

#include "gridex/rockpie/MqttHealthPublisher.hpp"

#include <string>
#include <vector>

namespace gridex::rockpie {
std::vector<SystemTelemetrySample> readSystemTelemetry(const std::string& dataDirectory,
                                                       const std::string& journalPath,
                                                       const std::string& cpuTemperaturePath,
                                                       bool includeCpuTemperature);
} // namespace gridex::rockpie
