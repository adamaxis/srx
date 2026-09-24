#pragma once

#include <cstdint>
#include <functional>

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

// ROM 0x144E8: enter the shared close-range state service boundary.
void apply_entity_close_range_state(const std::function<void()>& apply_state);

} // namespace shadowrun::ghidra_conversion
