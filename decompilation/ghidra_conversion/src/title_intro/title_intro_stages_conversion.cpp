#include "shadowrun_ghidra/title_intro/title_intro_stages_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

void service_audio(const TitleIntroStagesCallbacks& callbacks) {
    if (callbacks.service_audio) {
        callbacks.service_audio();
    }
}

bool stage_can_start(TitleIntroStagesState& state,
                     const TitleIntroStagesCallbacks& callbacks,
                     const std::function<void()>& stage) {
    if (callbacks.prepare_mode_entry) {
        callbacks.prepare_mode_entry();
    }
    service_audio(callbacks);
    state.stop_marker = 0;
    if (stage) {
        stage();
    }
    return state.stop_marker == 0;
}

} // namespace

TitleIntroStagesStopReason run_title_intro_stages(
    TitleIntroStagesState& state,
    const TitleIntroStagesCallbacks& callbacks) {
    state.controller_marker = 0xFFU;
    if (callbacks.sample_controller) {
        callbacks.sample_controller();
    }
    state.stop_marker = 0;
    service_audio(callbacks);
    if (callbacks.run_schedule) {
        callbacks.run_schedule();
    }

    bool completed = state.stop_marker == 0;
    if (completed) {
        completed = stage_can_start(state, callbacks, callbacks.run_stage2);
    }
    if (completed) {
        completed = stage_can_start(state, callbacks,
                                    callbacks.run_orchestrator);
    }
    if (completed) {
        state.stop_marker = 0;
        if (callbacks.run_stage_loop) {
            callbacks.run_stage_loop();
        }
        // The source clears this marker without testing it before the final
        // palette transition and post-intro entry.
        state.stop_marker = 0;
        if (callbacks.step_palette_transition) {
            callbacks.step_palette_transition();
        }
        state.completion_mode = 2U;
        if (callbacks.enter_post_intro_mode) {
            callbacks.enter_post_intro_mode();
        }
    }

    if (callbacks.sample_controller) {
        callbacks.sample_controller();
    }
    if (callbacks.step_palette_transition) {
        callbacks.step_palette_transition();
    }
    if (callbacks.prepare_mode_entry) {
        callbacks.prepare_mode_entry();
    }
    return completed ? TitleIntroStagesStopReason::source_complete
                     : TitleIntroStagesStopReason::interrupted;
}

} // namespace shadowrun::ghidra_conversion
