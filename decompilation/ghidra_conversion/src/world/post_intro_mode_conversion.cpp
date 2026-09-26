#include "shadowrun_ghidra/world/post_intro_mode_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

void service_frame(PostIntroModeState& state,
                   const PostIntroModeCallbacks& callbacks) {
    if (callbacks.wait_for_vblank) {
        callbacks.wait_for_vblank();
    }
    if (callbacks.latch_controller_events) {
        callbacks.latch_controller_events();
    }
    if (callbacks.sample_controller_input) {
        callbacks.sample_controller_input();
    }
    if (state.title_state == 0U && callbacks.service_world_state) {
        callbacks.service_world_state();
    }
}

void finalize_mode_transition(PostIntroModeState& state,
                              const PostIntroModeCallbacks& callbacks) {
    if (!state.menu_transition_pending &&
        state.selected_mode != state.current_mode) {
        if (callbacks.initialize_scene_mode_state) {
            callbacks.initialize_scene_mode_state();
        }
        if (callbacks.prepare_mode_entry) {
            callbacks.prepare_mode_entry();
        }
    }
}

} // namespace

PostIntroModeResult run_post_intro_mode(
    PostIntroModeState& state,
    const PostIntroModeCallbacks& callbacks) {
    if (callbacks.resolve_initial_mode) {
        state.selected_mode = callbacks.resolve_initial_mode(state.mode_family);
    }
    if (callbacks.decode_and_upload_source_stream) {
        callbacks.decode_and_upload_source_stream();
    }
    if (callbacks.render_hud_sprite_stream) {
        callbacks.render_hud_sprite_stream();
    }
    service_frame(state, callbacks);

    if (state.alternate_branch) {
        state.selected_mode = 4U;
        if (callbacks.render_mode_selection) {
            callbacks.render_mode_selection(state.selected_mode);
        }
        for (state.alternate_first_frame = 0;
             state.alternate_first_frame <= 0x0040U;
             ++state.alternate_first_frame) {
            service_frame(state, callbacks);
            if (state.stop_marker != 0U) {
                state.title_state = 0;
                return PostIntroModeResult::external_stop;
            }
        }

        state.selected_mode = 2U;
        if (callbacks.render_mode_selection) {
            callbacks.render_mode_selection(state.selected_mode);
        }
        for (state.alternate_second_frame = 0;
             state.alternate_second_frame <= 0x0060U;
             ++state.alternate_second_frame) {
            service_frame(state, callbacks);
            if (state.stop_marker != 0U) {
                state.title_state = 0;
                return PostIntroModeResult::external_stop;
            }
        }
        state.title_state = 0;
        return PostIntroModeResult::alternate_branch_complete;
    }

    if (callbacks.render_mode_selection) {
        callbacks.render_mode_selection(state.selected_mode);
    }
    if (!callbacks.read_controller_events) {
        return PostIntroModeResult::external_stop;
    }

    for (;;) {
        service_frame(state, callbacks);
        const auto events = callbacks.read_controller_events();
        if ((events & kPostIntroLeftEvents) != 0U) {
            if (state.selected_mode == 0U) {
                state.selected_mode = static_cast<std::uint8_t>(
                    kPostIntroModeCount - 1U);
            } else {
                --state.selected_mode;
            }
            if (callbacks.render_mode_selection) {
                callbacks.render_mode_selection(state.selected_mode);
            }
        }
        if ((events & kPostIntroRightEvents) != 0U) {
            ++state.selected_mode;
            if (state.selected_mode >= kPostIntroModeCount) {
                state.selected_mode = 0U;
            }
            if (callbacks.render_mode_selection) {
                callbacks.render_mode_selection(state.selected_mode);
            }
        }
        if ((events & kPostIntroExitEvents) != 0U) {
            if (callbacks.handle_exit_event) {
                callbacks.handle_exit_event();
            }
            state.exit_marker_written = true;
            if (callbacks.finalize_mode_exit) {
                callbacks.finalize_mode_exit();
            }
            finalize_mode_transition(state, callbacks);
            return PostIntroModeResult::returned_after_exit;
        }
        if ((events & kPostIntroActionEvent) != 0U &&
            callbacks.handle_action_event) {
            callbacks.handle_action_event();
        }
        if ((events & kPostIntroTransferEvent) != 0U) {
            if (callbacks.transfer_menu_asset_batch) {
                callbacks.transfer_menu_asset_batch();
            }
            finalize_mode_transition(state, callbacks);
            return PostIntroModeResult::returned_after_transfer;
        }
    }
}

} // namespace shadowrun::ghidra_conversion
