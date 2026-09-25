#include "shadowrun_ghidra/title_sweep_animation_conversion.hpp"

namespace shadowrun::ghidra_conversion {

TitleSweepAnimationAction advance_title_sweep_animation(
    TitleSweepAnimationState& state,
    const TitleSweepAnimationCallbacks& callbacks) {
    state.frame_delay = static_cast<std::int8_t>(state.frame_delay - 1);
    if (state.frame_delay >= 0) {
        return TitleSweepAnimationAction::waiting;
    }

    ++state.phase;
    if (state.phase >= 0x20U) {
        state.record_flags = static_cast<std::uint8_t>(
            state.record_flags & ~0x09U);
        if (callbacks.attach_behavior_descriptor_b) {
            callbacks.attach_behavior_descriptor_b();
        }
        state.transition_state = 0U;
        if (callbacks.return_to_title_state) {
            callbacks.return_to_title_state();
        }
        return TitleSweepAnimationAction::finalized;
    }

    const auto frame = callbacks.resolve_frame
                           ? callbacks.resolve_frame(state.phase)
                           : std::nullopt;
    if (!frame.has_value()) {
        return TitleSweepAnimationAction::frame_unresolved;
    }
    state.frame_delay = static_cast<std::int8_t>(frame->next_delay);
    if ((state.record_flags & 0x08U) != 0U) {
        return TitleSweepAnimationAction::waiting;
    }
    if (callbacks.build_sprite) {
        callbacks.build_sprite(TitleSweepSpriteBuild{
            frame->source_table_token, state.phase,
            static_cast<std::uint16_t>(state.base_x + frame->horizontal_offset),
            state.base_y, 0x6000U, frame->sprite_data_token});
        callbacks.build_sprite(TitleSweepSpriteBuild{
            frame->source_table_token, state.phase,
            static_cast<std::uint16_t>(state.base_x - frame->horizontal_offset),
            state.base_y, 0x6000U, frame->sprite_data_token});
    }
    return TitleSweepAnimationAction::rendered;
}

} // namespace shadowrun::ghidra_conversion
