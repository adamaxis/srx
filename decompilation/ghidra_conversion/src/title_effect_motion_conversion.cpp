#include "shadowrun_ghidra/title_effect_motion_conversion.hpp"

namespace shadowrun::ghidra_conversion {

TitleEffectMotionAction advance_title_effect_motion(
    TitleEffectMotionState& state,
    const TitleEffectMotionCallbacks& callbacks) {
    state.frame_delay = static_cast<std::uint16_t>(state.frame_delay - 1U);
    if ((state.frame_delay & 0x8000U) == 0U) {
        return TitleEffectMotionAction::waiting;
    }
    state.frame_delay = 3U;
    ++state.frame;
    if (state.frame >= 0x0CU) {
        if (state.position_x < 0x0120U) {
            state.first_source_token = 0x0001C288U;
            state.first_phase = 0U;
            if (callbacks.publish_source_record) {
                callbacks.publish_source_record(0x0001C288U, 0U);
            }
        } else {
            state.second_source_token = 0x0001C28EU;
            state.second_phase = 0U;
            if (callbacks.publish_source_record) {
                callbacks.publish_source_record(0x0001C28EU, 0U);
            }
        }
        state.transition_state = 0U;
        if (callbacks.run_menu_selection_service) {
            callbacks.run_menu_selection_service();
        }
        state.menu_state = 0U;
        return TitleEffectMotionAction::finalized;
    }

    const auto sprite = callbacks.resolve_sprite_data
                            ? callbacks.resolve_sprite_data(
                                  state.effect_mode, state.frame,
                                  state.position_x)
                            : std::nullopt;
    if (!sprite.has_value()) {
        return TitleEffectMotionAction::sprite_data_unresolved;
    }
    if (callbacks.build_sprite) {
        callbacks.build_sprite(*sprite, state.position_x, state.position_y,
                               0x2000U);
    }
    return TitleEffectMotionAction::rendered;
}

} // namespace shadowrun::ghidra_conversion
