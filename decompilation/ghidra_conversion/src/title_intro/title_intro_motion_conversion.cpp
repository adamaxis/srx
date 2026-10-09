#include "shadowrun_ghidra/title_intro/title_intro_motion_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void reset_title_intro_motion(TitleIntroMotionState& state) {
    state.next_event_frame = 0;
    state.phase = 0;
}

TitleIntroMotionAdvanceResult advance_title_intro_motion(
    TitleIntroMotionState& state,
    const std::array<TitleIntroMotionPhase, kTitleIntroMotionPhaseCount>&
        phases,
    const std::uint16_t scroll_accumulator,
    const TitleIntroMotionCallbacks& callbacks) {
    if (state.next_event_frame <= state.current_frame) {
        if (state.phase > 4U) {
            return TitleIntroMotionAdvanceResult::no_op;
        }

        const auto& phase = phases[state.phase];
        state.sprite_descriptor = phase.sprite_descriptor;
        state.next_event_frame = static_cast<std::uint16_t>(
            state.current_frame + phase.delay_frames);
        if (phase.position_if_nonzero != 0U) {
            state.position = phase.position_if_nonzero;
        }
        state.position_step = phase.position_step;
        ++state.phase;
        if (state.phase > 4U) {
            state.phase = 1U;
            const auto elapsed = static_cast<std::uint16_t>(
                state.current_frame - state.animation_start_frame);
            if (elapsed > 0x0167U) {
                state.phase = 5U;
                return TitleIntroMotionAdvanceResult::sequence_finished;
            }
        }
    }

    state.position = static_cast<std::uint16_t>(
        state.position + state.position_step);
    if (callbacks.draw_motion_sprite) {
        callbacks.draw_motion_sprite(
            {state.sprite_descriptor,
             static_cast<std::uint16_t>(state.position +
                                        (scroll_accumulator >> 8U)),
             0x0133U,
             0x6000U});
    }
    return TitleIntroMotionAdvanceResult::sprite_rendered;
}

void reset_title_intro_dual_sprites(TitleIntroDualSpriteState& state) {
    state.next_event_frame = 0;
    state.phase = 0;
    state.active = true;
}

void stop_title_intro_dual_sprites(TitleIntroDualSpriteState& state) {
    state.active = false;
}

void advance_title_intro_dual_sprites(
    TitleIntroDualSpriteState& state,
    const std::array<std::uint32_t, kTitleIntroDualSpriteCount>& sprites,
    const TitleIntroDualSpriteCallbacks& callbacks) {
    if (!state.active || state.phase >= 4U) {
        return;
    }

    if (state.next_event_frame <= state.current_frame) {
        state.left_sprite_descriptor = sprites[state.phase];
        const auto right_index = static_cast<std::size_t>(state.phase + 4U);
        state.right_sprite_descriptor = sprites[right_index];
        state.next_event_frame = static_cast<std::uint16_t>(
            state.current_frame + 6U);
        state.phase = static_cast<std::uint16_t>((state.phase + 1U) % 4U);
    }

    if (callbacks.draw_left_sprite) {
        callbacks.draw_left_sprite(
            {state.left_sprite_descriptor, 0x0183U, 0x00BEU, 0x6000U});
    }
    if (callbacks.draw_right_sprite) {
        callbacks.draw_right_sprite(
            {state.right_sprite_descriptor, 0x01ABU, 0x00D7U, 0x6000U});
    }
}

} // namespace shadowrun::ghidra_conversion
