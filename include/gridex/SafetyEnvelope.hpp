#pragma once

#include "gridex/Types.hpp"

namespace gridex {

class SafetyEnvelope {
public:
    [[nodiscard]] SafetyResult apply(const SafetyInput& input) const;
};

}  // namespace gridex

