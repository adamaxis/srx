#include "shadowrun_ghidra/entities/animation_reset_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void reset_entity_animation_state(EntityAnimationResetState& state) {
    state.frame_timer = 0;
    state.render_flags = static_cast<std::uint8_t>(
        state.render_flags | 0x01U);
    state.frame_phase = 0;
}

} // namespace shadowrun::ghidra_conversion
