#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

enum class WorldActivityMode : std::uint8_t {
    field = 1,
    resource_activity = 2,
    scripted_activity = 3,
    halted = 4,
};

} // namespace shadowrun::ghidra_conversion
