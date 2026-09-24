#include "shadowrun_ghidra/title_intro_persistent_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void reset_title_intro_persistent_sequence(
    TitleIntroPersistentSequenceState& state) {
    state.next_event_frame = 0;
    state.stage = 0;
}

TitleIntroPersistentAdvanceResult advance_title_intro_persistent_sequence(
    TitleIntroPersistentSequenceState& state,
    const std::array<TitleIntroPersistentPhase,
                     kTitleIntroPersistentPhaseCount>& phases,
    const TitleIntroPersistentCallbacks& callbacks) {
    if (state.stage >= 0x000CU) {
        return TitleIntroPersistentAdvanceResult::no_op;
    }

    const auto stage_before = state.stage;
    const bool event_due = state.next_event_frame <= state.current_frame;
    if (event_due) {
        if (state.stage < kTitleIntroPersistentPhaseCount) {
            const auto& phase = phases[state.stage];
            state.current_sprite_descriptor = phase.sprite_descriptor;
            state.next_event_frame = static_cast<std::uint16_t>(
                state.current_frame + phase.delay_frames);
            ++state.stage;
        } else if (state.stage == 4U) {
            if (callbacks.play_audio_event) {
                callbacks.play_audio_event(0x2FU);
            }
            state.related_phase_marker = 4U;
            if (callbacks.run_stage_four_transition) {
                callbacks.run_stage_four_transition(1U, 0x0EEEU);
            }
            ++state.stage;
            state.next_event_frame = static_cast<std::uint16_t>(
                state.current_frame + 9U);
        } else {
            if (callbacks.run_repeated_stage) {
                callbacks.run_repeated_stage();
            }
            state.next_event_frame = static_cast<std::uint16_t>(
                state.current_frame + 9U);
            ++state.stage;
            if (state.stage == 9U && callbacks.set_stage_nine_anchor) {
                callbacks.set_stage_nine_anchor(0x0182U, 0x00E2U);
            }
            if (state.stage >= 0x000CU && callbacks.draw_final_overlay) {
                callbacks.draw_final_overlay({0x03B8U, 0x0009U, 0x0008U});
            }
        }
    }

    if (callbacks.draw_persistent_sprite) {
        callbacks.draw_persistent_sprite(
            {state.current_sprite_descriptor, 0x0178U, 0x00D1U, 0x2000U});
    }
    if (state.stage >= 0x000CU) {
        return TitleIntroPersistentAdvanceResult::final_overlay_advanced;
    }
    if (!event_due) {
        return TitleIntroPersistentAdvanceResult::no_op;
    }
    if (stage_before < kTitleIntroPersistentPhaseCount) {
        return TitleIntroPersistentAdvanceResult::phase_advanced;
    }
    return TitleIntroPersistentAdvanceResult::repeated_stage_advanced;
}

} // namespace shadowrun::ghidra_conversion
