#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct EntityAnimationState {
    std::uint16_t frame = 0;
    std::int16_t accumulator = 0;
    std::int16_t increment = 0;
    std::int16_t period = 0;
    bool redraw_requested = false;
};

// ROM 0x5FDC: add one animation increment, advance the frame once when the
// period is reached, and request a redraw for that frame transition.
void advance_entity_animation_accumulator(EntityAnimationState& state);

} // namespace shadowrun::ghidra_conversion
