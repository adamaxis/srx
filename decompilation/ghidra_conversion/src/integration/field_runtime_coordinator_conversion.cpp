#include "shadowrun_ghidra/integration/field_runtime_coordinator_conversion.hpp"

namespace shadowrun::ghidra_conversion::integration {
namespace {

bool has_normal_frame_callbacks(
    const FieldRuntimeCoordinatorCallbacks& callbacks) {
    return static_cast<bool>(callbacks.latch_controller_events) &&
           static_cast<bool>(callbacks.sample_controller_input) &&
           static_cast<bool>(callbacks.service_entity_behavior_frames) &&
           static_cast<bool>(callbacks.update_field_camera) &&
           static_cast<bool>(callbacks.update_entity_render_pass) &&
           static_cast<bool>(callbacks.advance_palette_fade) &&
           static_cast<bool>(callbacks.advance_world_frame_logic);
}

FieldRuntimeFrameResult complete_field_scene_reentry(
    FieldRuntimeCoordinatorState& state,
    const FieldRuntimeCoordinatorCallbacks& callbacks) {
    if (!callbacks.advance_field_scene_transition) {
        return FieldRuntimeFrameResult::field_transition_pending;
    }

    // 0x0A54 and 0x09F0 publish both source markers before entering 0x2BD2.
    state.display_service_word = 0xFFFFU;
    state.scene_refresh_marker = -1;
    callbacks.advance_field_scene_transition();
    return FieldRuntimeFrameResult::field_transition_dispatched;
}

FieldRuntimeFrameResult dispatch_world_activity_reentry(
    FieldRuntimeCoordinatorState& state,
    const FieldRuntimeCoordinatorCallbacks& callbacks) {
    // 0x0A8A..0x0A98 performs the request service even when the caller came
    // through FFDF40 rather than an already-set FFDF42 request. Both leaves
    // are required before the source clears the request and re-enters 0x08C0.
    if (!callbacks.handle_world_activity_request ||
        !callbacks.initialize_field_scene_state) {
        return FieldRuntimeFrameResult::world_activity_pending;
    }

    if (state.world_activity_transition_marker != 0U) {
        state.world_activity_transition_marker = 0U;
        state.world_activity_scene_reset_bit = true;
    }
    state.display_service_word = 0xFFFFU;
    callbacks.handle_world_activity_request(state);
    callbacks.initialize_field_scene_state(state);
    state.world_activity_requested = false;
    return FieldRuntimeFrameResult::world_activity_scene_reentry_requested;
}

FieldRuntimeFrameResult dispatch_field_transition_request(
    FieldRuntimeCoordinatorState& state,
    const FieldRuntimeCoordinatorCallbacks& callbacks) {
    if (!callbacks.prepare_mode_entry_service) {
        return FieldRuntimeFrameResult::field_transition_pending;
    }

    // The source clears FFDF3E before it enters the mode-entry service. Keep
    // the request recoverable when a later converted leaf is not available;
    // an unresolved conversion must not disappear from the host state.
    state.field_transition_requested = false;
    state.mode_entry_marker = -1;
    callbacks.prepare_mode_entry_service();

    // 0x097C..0x0A32: a nonzero scene-runtime state changes to state 2 and
    // takes the same world-activity initialization tail as the fallback.
    if (state.scene_runtime_state != 0U) {
        if (!callbacks.initialize_world_activity_mode) {
            state.field_transition_requested = true;
            return FieldRuntimeFrameResult::field_transition_pending;
        }
        state.scene_runtime_state = 2U;
        callbacks.initialize_world_activity_mode(state);
        if (state.scene_id == 0x37U) {
            if (!callbacks.begin_scene_37_world_activity) {
                state.field_transition_requested = true;
                return FieldRuntimeFrameResult::field_transition_pending;
            }
            callbacks.begin_scene_37_world_activity(state);
        }
        return complete_field_scene_reentry(state, callbacks);
    }

    // 0x0992 routes an already-prepared world-activity scene directly into
    // the 0x0A72 request/scene-state branch.
    if (state.field_scene_gate != 0U) {
        const auto result = dispatch_world_activity_reentry(state, callbacks);
        if (result == FieldRuntimeFrameResult::world_activity_pending) {
            state.field_transition_requested = true;
        }
        return result;
    }

    if (!callbacks.toggle_field_scene_variant) {
        state.field_transition_requested = true;
        return FieldRuntimeFrameResult::field_transition_pending;
    }

    // 0x03168 uses carry to bypass the exit/mode-selection work and proceed
    // directly to the common display/scene-transition tail.
    if (callbacks.toggle_field_scene_variant(state)) {
        return complete_field_scene_reentry(state, callbacks);
    }

    if (!callbacks.run_field_exit_orchestrator) {
        state.field_transition_requested = true;
        return FieldRuntimeFrameResult::field_transition_pending;
    }
    const bool entity_anchor_refresh_required =
        callbacks.run_field_exit_orchestrator(state);
    state.field_activity_lock = false;

    // 0x09B2 tests the exit-orchestrator's caller-visible D6 result. A
    // nonzero result resets the six runner transition words, snapshots the
    // anchors, and then enters 0x2BD2 without selecting another mode.
    if (entity_anchor_refresh_required) {
        if (!callbacks.reset_field_entity_transition_words ||
            !callbacks.snapshot_entity_anchors) {
            state.field_transition_requested = true;
            return FieldRuntimeFrameResult::field_transition_pending;
        }
        callbacks.reset_field_entity_transition_words(state);
        state.display_service_word = 0xFFFFU;
        callbacks.snapshot_entity_anchors(state);
        return complete_field_scene_reentry(state, callbacks);
    }

    // 0x09FA..0x0A0E is the one scene/submode combination that skips both
    // mode transition and world-activity initialization.
    if (state.scene_id == 4U && state.field_mode == 6U) {
        return complete_field_scene_reentry(state, callbacks);
    }

    // 0x0A12..0x0A2E selects the mode-transition table only when FFFC0E is
    // set and the field mode is at least 8; otherwise it initializes the
    // world-activity mode.
    if (state.mode_transition_gate != 0U && state.field_mode >= 8U) {
        if (!callbacks.advance_field_mode_transition) {
            state.field_transition_requested = true;
            return FieldRuntimeFrameResult::field_transition_pending;
        }
        callbacks.advance_field_mode_transition(state);
    } else {
        if (!callbacks.initialize_world_activity_mode) {
            state.field_transition_requested = true;
            return FieldRuntimeFrameResult::field_transition_pending;
        }
        callbacks.initialize_world_activity_mode(state);
        if (state.scene_id == 0x37U) {
            if (!callbacks.begin_scene_37_world_activity) {
                state.field_transition_requested = true;
                return FieldRuntimeFrameResult::field_transition_pending;
            }
            callbacks.begin_scene_37_world_activity(state);
        }
    }

    return complete_field_scene_reentry(state, callbacks);
}

} // namespace

FieldRuntimeFrameResult advance_field_runtime_frame(
    FieldRuntimeCoordinatorState& state,
    const FieldRuntimeCoordinatorCallbacks& callbacks) {
    state.action_event_dispatched = false;
    state.command_overview_dispatched = false;
    state.pending_handler_dispatched = false;

    if (!callbacks.wait_for_vblank) {
        return FieldRuntimeFrameResult::callback_unresolved;
    }
    callbacks.wait_for_vblank();

    // ROM 0x095E checks FFDF3E before sampling input or running a normal
    // entity frame.  The source clears the request before dispatching the
    // converted transition branch.
    if (state.field_transition_requested) {
        return dispatch_field_transition_request(state, callbacks);
    }

    // ROM 0x0A68 gives FFDF42 the next priority.  The source clears this
    // request after 0x1E43C/0x2F32 complete; the child conversion owns the
    // exact world-activity and scene-state writes.
    if (state.world_activity_requested) {
        return dispatch_world_activity_reentry(state, callbacks);
    }

    if (!has_normal_frame_callbacks(callbacks)) {
        return FieldRuntimeFrameResult::callback_unresolved;
    }

    // ROM 0x0AA8..0x0ACC.  This order matters: the sampled controller word
    // is consumed only after the latch, and world-frame logic follows palette
    // fading rather than preceding it.
    callbacks.latch_controller_events();
    state.controller_word = callbacks.sample_controller_input();
    callbacks.service_entity_behavior_frames();
    callbacks.update_field_camera();
    callbacks.update_entity_render_pass();
    callbacks.advance_palette_fade();
    callbacks.advance_world_frame_logic();
    state.display_dirty_flags = static_cast<std::uint8_t>(
        state.display_dirty_flags | 0x0EU);

    // ROM 0x0AEA..0x0B04 suppresses action/command dispatch while an activity
    // lock, activity countdown, or display service is still active.
    if (state.field_activity_lock || state.world_activity_countdown != 0U ||
        state.display_service_word != 0U) {
        return FieldRuntimeFrameResult::frame_waiting;
    }

    // ROM 0x0B08 checks the action bit first.  It intentionally does not
    // return, so a sampled word containing both bits can also enter the
    // command-overview branch below.
    if ((state.controller_word & 0x2000U) != 0U) {
        if (!callbacks.handle_mode_action_event) {
            return FieldRuntimeFrameResult::callback_unresolved;
        }
        callbacks.handle_mode_action_event();
        state.action_event_dispatched = true;
    }

    if ((state.controller_word & 0x8000U) != 0U) {
        if (!callbacks.run_fixed_audio_transaction ||
            !callbacks.run_command_overview || !callbacks.service_audio_tick ||
            !callbacks.select_next_entity) {
            return FieldRuntimeFrameResult::callback_unresolved;
        }
        callbacks.run_fixed_audio_transaction(8U);
        state.display_service_word = 0xFFFFU;
        callbacks.run_command_overview();
        callbacks.service_audio_tick();
        callbacks.select_next_entity();
        state.command_overview_dispatched = true;

        if (state.joshua_archetype_selection_pending) {
            if (!callbacks.restart_field_session) {
                return FieldRuntimeFrameResult::field_session_restart_pending;
            }
            callbacks.restart_field_session();
            return FieldRuntimeFrameResult::field_session_restart_requested;
        }
        if (!callbacks.advance_field_scene_transition) {
            return FieldRuntimeFrameResult::callback_unresolved;
        }
        callbacks.advance_field_scene_transition();
        return FieldRuntimeFrameResult::command_overview_dispatched;
    }

    if (!state.pending_field_scene_handler) {
        return state.action_event_dispatched
                   ? FieldRuntimeFrameResult::action_event_dispatched
                   : FieldRuntimeFrameResult::frame_waiting;
    }
    if (!callbacks.run_pending_field_scene_handler) {
        return FieldRuntimeFrameResult::callback_unresolved;
    }

    state.display_service_word = 0xFFFFU;
    state.pending_handler_dispatched = true;
    const auto handler_result = callbacks.run_pending_field_scene_handler();
    state.pending_field_scene_handler = false;
    if (handler_result == PendingFieldSceneHandlerResult::returned_to_title) {
        return FieldRuntimeFrameResult::returned_to_title;
    }
    if (!callbacks.advance_field_scene_transition) {
        return FieldRuntimeFrameResult::callback_unresolved;
    }
    callbacks.advance_field_scene_transition();
    return FieldRuntimeFrameResult::field_scene_reentry_requested;
}

} // namespace shadowrun::ghidra_conversion::integration
