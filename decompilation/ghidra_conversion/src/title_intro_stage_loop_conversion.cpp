#include "shadowrun_ghidra/title_intro_stage_loop_conversion.hpp"

namespace shadowrun::ghidra_conversion {

TitleIntroStageLoopStopReason run_title_intro_stage_loop(
    TitleIntroStageLoopState& state,
    const std::array<std::uint32_t, kTitleIntroPulsePhaseCount>&
        pulse_descriptors,
    const std::array<TitleIntroLandingPhase, kTitleIntroLandingPhaseCount>&
        landing_phases,
    std::uint32_t special_landing_descriptor,
    const TitleIntroStageLoopCallbacks& callbacks) {
    if (callbacks.clear_display_buffer) {
        callbacks.clear_display_buffer();
    }
    if (callbacks.prepare_title_resources) {
        callbacks.prepare_title_resources();
    }

    state.animation_start_frame = state.current_frame;
    state.pulse.current_frame = state.current_frame;
    state.landing.current_frame = state.current_frame;
    state.landing.animation_start_frame = state.animation_start_frame;
    reset_title_intro_scroll_fill(state.scroll);
    reset_title_intro_landing(state.landing);
    state.landing.animation_start_frame = state.animation_start_frame;
    service_title_intro_animation(
        state.animation_service, state.pulse, pulse_descriptors,
        state.landing, landing_phases, special_landing_descriptor,
        callbacks.draw_sprite);
    if (callbacks.render_middle_text) {
        callbacks.render_middle_text();
    }
    if (callbacks.restore_scene_records) {
        callbacks.restore_scene_records();
    }
    if (callbacks.sample_controller) {
        callbacks.sample_controller();
    }

    for (state.loop_frame = 0;
         state.loop_frame <= kTitleIntroStageLoopLastFrame;
         ++state.loop_frame) {
        if (callbacks.wait_for_vblank) {
            callbacks.wait_for_vblank();
        }
        if (callbacks.sample_controller) {
            callbacks.sample_controller();
        }
        if (callbacks.latch_controller_events) {
            callbacks.latch_controller_events();
        }
        if (state.loop_frame == kTitleIntroStageFinalTextFrame &&
            callbacks.render_final_text) {
            callbacks.render_final_text();
        }

        state.pulse.current_frame = state.current_frame;
        state.landing.current_frame = state.current_frame;
        advance_title_intro_triplet(state.triplet, state.current_frame);
        advance_title_intro_scroll(state.scroll, callbacks.fill_transition_tiles);
        advance_title_intro_scroll_fade(state.scroll,
                                        callbacks.render_transition_text);
        service_title_intro_animation(
            state.animation_service, state.pulse, pulse_descriptors,
            state.landing, landing_phases, special_landing_descriptor,
            callbacks.draw_sprite);
        if (callbacks.stop_requested && callbacks.stop_requested()) {
            return TitleIntroStageLoopStopReason::external_stop;
        }
    }
    return TitleIntroStageLoopStopReason::source_complete;
}

} // namespace shadowrun::ghidra_conversion
