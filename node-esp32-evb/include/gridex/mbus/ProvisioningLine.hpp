#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string>

namespace gridex::mbus {

// Discard the entire overlong line, including a suffix that looks like a command.
class ProvisioningLine {
public:
    std::optional<std::string> push(char value) {
        if (value == '\r' || value == '\n') {
            std::optional<std::string> result;
            if (!discarding_ && size_ != 0U) {
                result = std::string(buffer_.data(), size_);
            }
            size_ = 0;
            discarding_ = false;
            return result;
        }
        if (!discarding_) {
            if (size_ == buffer_.size()) {
                discarding_ = true;
            } else {
                buffer_[size_++] = value;
            }
        }
        return std::nullopt;
    }
private:
    std::array<char, 80> buffer_{};
    std::size_t size_{0};
    bool discarding_{false};
};
}  // namespace gridex::mbus
