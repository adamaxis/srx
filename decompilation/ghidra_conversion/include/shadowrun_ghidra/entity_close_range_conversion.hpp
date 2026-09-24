#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct EntityCloseRangeState {
    std::uint8_t flags = 0;
    std::uint8_t secondary_state = 0;
    std::uint32_t continuation = 0;
};

// ROM 0x1460E: arm the close-range entity state only below the source 0x1E0
// distance threshold.
bool apply_close_range_entity_state(
    EntityCloseRangeState& state,
    std::uint16_t distance);

} // namespace shadowrun::ghidra_conversion
