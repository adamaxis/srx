#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct EntityTimingState {
    std::uint8_t flags = 0;
    std::int8_t primary_timer = 0;
    std::int8_t secondary_timer = 0;
};

// ROM 0x4E38: advance the primary entity timer only on frame-counter phase
// zero, unless the source's 0x40 flag bypasses the timer entirely.
std::uint16_t advance_entity_primary_timer(EntityTimingState& state,
                                            std::uint32_t frame_counter,
                                            std::uint16_t inactive_result);

// ROM 0x4E64: advance the secondary entity timer on frame-counter phase zero.
std::uint16_t advance_entity_secondary_timer(EntityTimingState& state,
                                              std::uint32_t frame_counter);

} // namespace shadowrun::ghidra_conversion
