#include "shadowrun_ghidra/world_effect_scene_loop_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

template <typename Callback, typename... Args>
void call_if_present(const Callback& callback, Args&&... args) {
    if (callback) {
        callback(static_cast<Args&&>(args)...);
    }
}

WorldEffectSceneLoopResult result(const WorldEffectSceneExitReason reason,
                                  const std::size_t frames) {
    return WorldEffectSceneLoopResult{reason, frames};
}

} // namespace

WorldEffectSceneLoopResult run_world_effect_scene_loop(
    WorldEffectSceneLoopState& state,
    const WorldEffectSceneLoopCallbacks& callbacks,
    const std::size_t max_frames) {
    // 0x1ADE6 prologue.  These are scene-owned fields; source inputs such as
    // FFF0A0, FFFBCA, and the current runner footer remain caller-owned.
    state.work_x = 0x0120U;
    state.work_y = 0x00D0U;
    state.world_sequence_active = false;
    state.exit_requested = false;
    state.scene_complete = false;
    state.scene_completion_aux = 0U;
    state.title_phase_active = false;
    state.alternating_profile_active = false;
    state.audio_track = state.source_audio_track;
    state.audio_countdown = 0U;
    state.title_service_flags = 0U;
    state.world_effect_mode = 0U;
    state.burst_active = false;
    state.noise_effect_active = false;
    state.profile_transition_active = false;
    state.profile_motion_active = false;
    state.title_effect_motion_active = false;
    state.world_effect_motion_active = false;
    state.matrix_transition_active = false;
    state.title_animation_active = false;
    state.persistent_title_effect_active = false;
    state.persistent_world_effect_active = false;
    state.frame_presented = false;
    state.dirty_flags = 0U;

    call_if_present(callbacks.reset_word_stream_area);
    call_if_present(callbacks.upload_decoded_source_stream);
    call_if_present(callbacks.initialize_world_effect_mode);
    call_if_present(callbacks.attach_behavior_descriptor_e);

    std::size_t frames = 0U;
    for (;;) {
        if (max_frames != 0U && frames >= max_frames) {
            return result(WorldEffectSceneExitReason::frame_budget_exhausted,
                          frames);
        }

        // 0x1AEAC: one controller/title tick begins with the hardware-facing
        // frame services.  They are presentation/input boundaries here.
        call_if_present(callbacks.wait_for_vblank);
        call_if_present(callbacks.latch_controller_events);
        if (callbacks.sample_controller_input) {
            state.controller_events = callbacks.sample_controller_input();
        }
        call_if_present(callbacks.advance_menu_random_glitch);

        if (state.exit_requested && !state.world_sequence_active) {
            call_if_present(callbacks.initialize_world_sequence);
            state.world_sequence_active = true;
        }

        if (state.world_sequence_active) {
            const bool sequence_complete =
                callbacks.advance_world_sequence &&
                callbacks.advance_world_sequence();
            if (sequence_complete) {
                state.world_sequence_active = false;
                call_if_present(callbacks.prepare_mode_entry_service);
                call_if_present(callbacks.service_audio_command_frame, 0x0005U);
                if (state.runner_selection_mode) {
                    call_if_present(callbacks.run_runner_selection_screen);
                } else {
                    call_if_present(callbacks.run_mode_action_loop);
                }
                call_if_present(callbacks.prepare_mode_entry_service);
                return result(WorldEffectSceneExitReason::handed_to_mode,
                              frames);
            }
            call_if_present(callbacks.advance_profile_copy_timer);
        } else if (state.scene_complete) {
            const bool jitter_active = callbacks.advance_title_jitter &&
                                       callbacks.advance_title_jitter();
            if (!jitter_active) {
                state.scene_complete = false;
                call_if_present(callbacks.advance_hud_fade_service);
                call_if_present(callbacks.finish_title_fade);
                return result(WorldEffectSceneExitReason::title_fade_complete,
                              frames);
            }
        } else {
            // 0x1AF3E: a missing footer or an inactive runner footer starts
            // the scene-completion jitter path for the following frame.
            if (!state.footer_blocked || !state.current_runner_footer_visible) {
                state.scene_complete = true;
                state.scene_completion_aux = 0U;
            }

            call_if_present(callbacks.advance_world_menu_service);
            call_if_present(callbacks.advance_title_world_interaction_service);
            call_if_present(callbacks.handle_menu_directional_input);
            call_if_present(callbacks.advance_world_effect_controller);
            if (state.hud_abort) {
                return result(WorldEffectSceneExitReason::hud_abort, frames);
            }

            if (state.audio_countdown != 0U) {
                state.audio_countdown = static_cast<std::uint8_t>(
                    state.audio_countdown - 1U);
                if (state.audio_countdown == 0U) {
                    call_if_present(callbacks.service_audio_if_enabled,
                                    state.alternate_audio ? 0x001BU : 0x001AU);
                }
            }
            if (state.alternating_profile_active) {
                call_if_present(callbacks.advance_alternating_profile);
            } else {
                call_if_present(callbacks.advance_profile_copy_timer);
            }
        }

        state.frame_presented = true;
        if (state.world_sequence_active) {
            call_if_present(callbacks.advance_title_figure_animation);
        } else {
            if (state.world_effect_mode == 4U) {
                call_if_present(callbacks.append_title_vertical_sprite);
            }
            if (state.title_phase_active) {
                call_if_present(callbacks.advance_title_phase_cycle);
            }
            call_if_present(callbacks.advance_title_sprite_resource_service);
            call_if_present(callbacks.advance_title_sprite_layers);
            if ((state.title_service_flags & 0x01U) != 0U) {
                call_if_present(callbacks.advance_title_sweep_animation);
            }
            if ((state.title_service_flags & 0x02U) != 0U) {
                call_if_present(callbacks.advance_audio_menu_countdown);
            }
            if ((state.title_service_flags & 0x04U) != 0U) {
                call_if_present(callbacks.advance_title_noise_stream);
            }
            if ((state.title_service_flags & 0x10U) != 0U) {
                call_if_present(callbacks.advance_title_phase_service);
            }
            if ((state.title_service_flags & 0x40U) != 0U) {
                call_if_present(callbacks.advance_title_secondary_effect_service);
            }
            if ((state.title_service_flags & 0x20U) != 0U) {
                call_if_present(callbacks.dispatch_title_phase_handler);
            }
            if (state.burst_active) {
                call_if_present(callbacks.advance_title_burst_animation);
            }
            if (state.noise_effect_active) {
                call_if_present(callbacks.advance_title_noise_effect);
            }
            if (state.profile_transition_active) {
                call_if_present(callbacks.advance_title_profile_transition);
            }
            if (state.profile_motion_active) {
                call_if_present(callbacks.advance_title_profile_motion);
            }
            if (state.title_effect_motion_active) {
                call_if_present(callbacks.advance_title_effect_motion);
            }
            if (state.world_effect_motion_active) {
                call_if_present(callbacks.advance_world_effect_motion);
            }
            if (state.world_effect_mode != 0U) {
                call_if_present(callbacks.advance_world_effect_mode_animation);
            }
            if (state.title_animation_active) {
                call_if_present(callbacks.advance_title_animation_cycle);
            }
            if (state.matrix_transition_active) {
                call_if_present(callbacks.advance_matrix_program_transition);
            }
            if (state.persistent_title_effect_active) {
                call_if_present(callbacks.update_persistent_title_effect);
            }
            if (state.persistent_world_effect_active) {
                call_if_present(callbacks.advance_persistent_world_effect);
            }
        }

        if (!state.exit_requested) {
            call_if_present(callbacks.tick_world_effect_animation);
        }
        call_if_present(callbacks.append_world_sprite_records);

        // 0x1B144: field/menu refresh after the current sprite list has been
        // assembled.  The source bit is consumed as a branch condition; no
        // Genesis VDP command is reproduced in this host conversion.
        if (!state.world_sequence_active &&
            (state.controller_events & 0x8000U) != 0U) {
            call_if_present(callbacks.prepare_mode_entry_service);
            call_if_present(callbacks.service_audio_command_frame, 0x0004U);
            call_if_present(callbacks.reset_word_stream_area);
            call_if_present(callbacks.run_matrix_loop);
            if (!callbacks.refresh_field_resource_streams ||
                !callbacks.refresh_field_resource_streams()) {
                return result(
                    WorldEffectSceneExitReason::resource_refresh_stopped,
                    frames + 1U);
            }
            if (state.hud_abort) {
                return result(WorldEffectSceneExitReason::hud_abort,
                              frames + 1U);
            }
            call_if_present(callbacks.refresh_field_hud_assets);
            call_if_present(callbacks.prepare_menu_hud_sequence);
            call_if_present(callbacks.set_display_control, 0x8174U);
            call_if_present(callbacks.prepare_mode_entry_service);
            call_if_present(callbacks.service_audio_command_frame, 0x0003U);
        }

        state.dirty_flags = static_cast<std::uint8_t>(state.dirty_flags | 0x02U);
        ++frames;
    }
}

} // namespace shadowrun::ghidra_conversion
