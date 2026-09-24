#include "shadowrun_ghidra/title_intro_sequence_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void reset_title_intro_sequence(TitleIntroSequenceState& state) {
    state.next_event_frame = 0;
    state.stage = 0;
}

TitleIntroSequenceAdvanceResult advance_title_intro_sequence(
    TitleIntroSequenceState& state,
    const std::array<TitleIntroSequencePhase, kTitleIntroSequencePhaseCount>&
        phases,
    const TitleIntroSequenceCallbacks& callbacks) {
    if (state.stage >= 0x000CU || state.current_frame < state.next_event_frame) {
        return TitleIntroSequenceAdvanceResult::no_op;
    }

    if (state.stage < kTitleIntroSequencePhaseCount) {
        const auto& phase = phases[state.stage];
        state.next_event_frame = static_cast<std::uint16_t>(
            state.current_frame + phase.delay_frames);
        if (callbacks.render_sprite_stream) {
            callbacks.render_sprite_stream(
                {phase.sprite_descriptor, 0x0826U, 0x000CU, 0x000BU,
                 0x0000U, 0x0000U, 0x000DU});
        }
        ++state.stage;
        return TitleIntroSequenceAdvanceResult::phase_rendered;
    }

    if (state.stage == 4U) {
        if (callbacks.play_audio_event) {
            callbacks.play_audio_event(0x46U);
        }
        if (callbacks.run_stage_four_transition) {
            callbacks.run_stage_four_transition(0U, 0x0EEEU);
        }
        ++state.stage;
        state.next_event_frame = static_cast<std::uint16_t>(
            state.current_frame + 9U);
        return TitleIntroSequenceAdvanceResult::transition_rendered;
    }

    if (callbacks.run_repeated_stage) {
        callbacks.run_repeated_stage();
    }
    state.next_event_frame = static_cast<std::uint16_t>(
        state.current_frame + 9U);
    ++state.stage;
    if (state.stage == 9U && callbacks.set_stage_nine_anchor) {
        callbacks.set_stage_nine_anchor(0x013DU, 0x0139U);
    }
    if (state.stage >= 0x000CU && callbacks.draw_final_overlay) {
        callbacks.draw_final_overlay({0x0826U, 0x000CU, 0x000BU});
        return TitleIntroSequenceAdvanceResult::final_overlay_rendered;
    }
    return TitleIntroSequenceAdvanceResult::repeated_stage_rendered;
}

} // namespace shadowrun::ghidra_conversion
