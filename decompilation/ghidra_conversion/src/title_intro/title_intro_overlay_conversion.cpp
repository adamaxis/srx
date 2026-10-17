#include "shadowrun_ghidra/title_intro/title_intro_overlay_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void reset_title_intro_overlay_schedule(TitleIntroOverlaySchedule& schedule) {
    schedule.next_sprite_frame = 0;
    schedule.phase_index = 0;
    schedule.scroll_accumulator = 0;
}

TitleIntroOverlayAdvanceResult advance_title_intro_overlay(
    TitleIntroOverlayState& state,
    const std::array<TitleIntroOverlayPhase, kTitleIntroOverlayPhaseCount>&
        phases,
    const std::uint32_t final_overlay_descriptor,
    const TitleIntroOverlayCallbacks& callbacks) {
    const auto elapsed = static_cast<std::uint16_t>(
        state.current_frame - state.animation_start_frame);
    state.horizontal_scroll_written = false;
    if (elapsed > 0x0168U) {
        return TitleIntroOverlayAdvanceResult::no_op;
    }

    if (elapsed == 0x0168U) {
        if (callbacks.play_audio_event) {
            callbacks.play_audio_event(0x2EU);
        }
        state.horizontal_scroll = 0;
        state.horizontal_scroll_written = true;
        if (callbacks.write_transfer_header) {
            callbacks.write_transfer_header();
        }
        if (callbacks.clear_wilderness_left_region) {
            // ROM 0x59FBE-0x59FC6 fills the 17-by-16 left actor region with
            // the blank tile before the terminal death-pose stream is sent.
            callbacks.clear_wilderness_left_region();
        }
        if (callbacks.render_sprite_stream) {
            callbacks.render_sprite_stream(
                {final_overlay_descriptor, 0x0786U, 0x0013U, 0x000CU,
                 0x4000U, 0x0000U, 0x0014U});
        }
        return TitleIntroOverlayAdvanceResult::final_overlay_rendered;
    }

    if (elapsed >= 0x0120U) {
        state.schedule.scroll_accumulator = static_cast<std::uint16_t>(
            state.schedule.scroll_accumulator + 0x00AAU);
        state.horizontal_scroll = static_cast<std::uint16_t>(
            state.schedule.scroll_accumulator >> 8U);
        state.horizontal_scroll_written = true;
        state.dirty_flags = static_cast<std::uint8_t>(
            state.dirty_flags | 0x04U);
    }

    if (state.current_frame < state.schedule.next_sprite_frame) {
        return TitleIntroOverlayAdvanceResult::no_op;
    }

    const auto& phase = phases[state.schedule.phase_index];
    state.schedule.next_sprite_frame = static_cast<std::uint16_t>(
        state.current_frame + phase.delay_frames);
    if (callbacks.render_sprite_stream) {
        callbacks.render_sprite_stream(
            {phase.sprite_descriptor, 0x0600U, 0x0010U, 0x000FU, 0x4000U,
             0x0000U, 0x0011U});
    }
    state.schedule.phase_index = static_cast<std::uint16_t>(
        (state.schedule.phase_index + 1U) % kTitleIntroOverlayPhaseCount);
    return TitleIntroOverlayAdvanceResult::phase_rendered;
}

} // namespace shadowrun::ghidra_conversion
