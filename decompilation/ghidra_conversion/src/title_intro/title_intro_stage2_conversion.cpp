#include "shadowrun_ghidra/title_intro/title_intro_stage2_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

void advance_scene_entry_offsets(TitleIntroStage2State& state) {
    if (state.x_offset != 0) {
        if (state.x_phase != 0x0178U) {
            ++state.x_phase;
        }
        ++state.x_offset;
        state.dirty_flags = static_cast<std::uint8_t>(
            state.dirty_flags | 0x04U);
    }
    if (state.y_offset != 0 && (state.current_frame & 1U) == 0U) {
        if (state.y_phase != 0x00D1U) {
            --state.y_phase;
        }
        ++state.y_offset;
        state.dirty_flags = static_cast<std::uint8_t>(
            state.dirty_flags | 0x08U);
    }
}

void service_scene_frame(TitleIntroStage2State& state,
                         TitleIntroScheduleState& schedule_state,
                         const std::vector<TitleIntroScheduleEntry>& entries,
                         const TitleIntroStage2Callbacks& callbacks) {
    advance_title_intro_schedule(schedule_state, entries,
                                  callbacks.run_scheduled_action);
    advance_scene_entry_offsets(state);
    if (callbacks.initialize_scene_mode) {
        callbacks.initialize_scene_mode();
    }
    if (callbacks.render_scene_entities) {
        callbacks.render_scene_entities();
    }
}

} // namespace

TitleIntroStage2StopReason run_title_intro_stage2(
    TitleIntroStage2State& state,
    TitleIntroScheduleState& schedule_state,
    const std::vector<TitleIntroScheduleEntry>& schedule_entries,
    const TitleIntroStage2Callbacks& callbacks) {
    if (callbacks.load_title_font) {
        callbacks.load_title_font();
    }
    schedule_state.current_entry = 0;
    schedule_state.countdown = schedule_entries.empty()
                                   ? 0
                                   : schedule_entries.front().delay_frames;
    if (callbacks.prepare_title_resources) {
        callbacks.prepare_title_resources();
    }
    state.x_offset = -0x0040;
    state.y_offset = -0x0020;
    state.x_phase = 0x0138;
    state.y_phase = 0x00F1;
    state.dirty_flags = static_cast<std::uint8_t>(
        state.dirty_flags | 0x0CU);
    service_scene_frame(state, schedule_state, schedule_entries, callbacks);
    if (callbacks.advance_palette_fade) {
        callbacks.advance_palette_fade();
    }

    for (state.first_loop_frame = 0; state.first_loop_frame <= 0x00A0U;
         ++state.first_loop_frame) {
        if (callbacks.wait_for_vblank) {
            callbacks.wait_for_vblank();
        }
        if (callbacks.sample_controller) {
            callbacks.sample_controller();
        }
        if (callbacks.latch_controller_events) {
            callbacks.latch_controller_events();
        }
        service_scene_frame(state, schedule_state, schedule_entries,
                            callbacks);
        if (callbacks.stop_requested && callbacks.stop_requested()) {
            return TitleIntroStage2StopReason::external_stop;
        }
    }

    state.animation_start_frame = state.current_frame;
    if (callbacks.reset_overlay) {
        callbacks.reset_overlay();
    }
    if (callbacks.reset_sequence) {
        callbacks.reset_sequence();
    }
    if (callbacks.reset_persistent_sequence) {
        callbacks.reset_persistent_sequence();
    }
    if (callbacks.reset_motion) {
        callbacks.reset_motion();
    }
    if (callbacks.reset_dual_sprites) {
        callbacks.reset_dual_sprites();
    }
    if (callbacks.reset_timed_sprites) {
        callbacks.reset_timed_sprites();
    }
    if (callbacks.reset_targeted_sweep) {
        callbacks.reset_targeted_sweep();
    }
    if (callbacks.reset_linear_sweep_a) {
        callbacks.reset_linear_sweep_a();
    }
    if (callbacks.reset_linear_sweep_b) {
        callbacks.reset_linear_sweep_b();
    }
    if (callbacks.reset_final_projection) {
        callbacks.reset_final_projection();
    }
    if (callbacks.initialize_ambush) {
        callbacks.initialize_ambush();
    }
    if (callbacks.sample_controller) {
        callbacks.sample_controller();
    }

    for (state.second_loop_frame = 0;
         state.second_loop_frame <= 0x01F0U; ++state.second_loop_frame) {
        if (callbacks.wait_for_vblank) {
            callbacks.wait_for_vblank();
        }
        if (callbacks.advance_overlay) {
            callbacks.advance_overlay();
        }
        if (callbacks.advance_sequence) {
            callbacks.advance_sequence();
        }
        if (callbacks.sample_controller) {
            callbacks.sample_controller();
        }
        if (callbacks.latch_controller_events) {
            callbacks.latch_controller_events();
        }
        service_scene_frame(state, schedule_state, schedule_entries,
                            callbacks);
        if (callbacks.advance_ambush) {
            callbacks.advance_ambush();
        }
        if (state.second_loop_frame == 0x0164U) {
            if (callbacks.render_scroll_text) {
                callbacks.render_scroll_text();
            }
            if (callbacks.render_late_text) {
                callbacks.render_late_text();
            }
        }
        if (callbacks.stop_requested && callbacks.stop_requested()) {
            return TitleIntroStage2StopReason::external_stop;
        }
    }
    return TitleIntroStage2StopReason::source_complete;
}

} // namespace shadowrun::ghidra_conversion
