#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct EntityAnimationResetState {
    std::uint16_t frame_timer = 0;
    std::uint8_t render_flags = 0;
    std::uint16_t frame_phase = 0;
};

// ROM 0xAF6C: restart the entity animation phase and request the next render
// pass. This is the shared reset used after heading and behavior changes.
void reset_entity_animation_state(EntityAnimationResetState& state);

} // namespace shadowrun::ghidra_conversion
