#include "shadowrun_ghidra/world_effect_motion_conversion.hpp"

namespace shadowrun::ghidra_conversion {

WorldEffectMotionAction advance_world_effect_motion(
    WorldEffectMotionState& state,
    const WorldEffectMotionCallbacks& callbacks) {
    state.frame_countdown = static_cast<std::uint16_t>(
        state.frame_countdown - 1U);
    if ((state.frame_countdown & 0x8000U) != 0U) {
        state.completion_marker = 0U;
        state.transition_state = 0U;
        if (callbacks.run_menu_selection_service) {
            callbacks.run_menu_selection_service();
        }
        return WorldEffectMotionAction::finalized;
    }

    ++state.motion_phase;
    const auto sprite = callbacks.resolve_sprite_data
                            ? callbacks.resolve_sprite_data(
                                  state.motion_phase,
                                  state.horizontal_offset < 0)
                            : std::nullopt;
    if (!sprite.has_value()) {
        return WorldEffectMotionAction::sprite_data_unresolved;
    }
    state.position_x = static_cast<std::uint16_t>(
        state.position_x + state.horizontal_offset);
    state.position_y = static_cast<std::uint16_t>(state.position_y - 0x001AU);
    if (callbacks.build_sprite) {
        callbacks.build_sprite(*sprite,
                               static_cast<std::uint16_t>(
                                   state.position_x >> 4U),
                               static_cast<std::uint16_t>(
                                   state.position_y >> 4U),
                               0x2000U);
    }
    return WorldEffectMotionAction::rendered;
}

} // namespace shadowrun::ghidra_conversion
