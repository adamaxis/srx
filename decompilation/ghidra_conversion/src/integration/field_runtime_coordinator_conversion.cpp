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
        if (!callbacks.handle_field_transition_request) {
            return FieldRuntimeFrameResult::field_transition_pending;
        }
        state.field_transition_requested = false;
        callbacks.handle_field_transition_request(state);
        return FieldRuntimeFrameResult::field_transition_dispatched;
    }

    // ROM 0x0A68 gives FFDF42 the next priority.  The source clears this
    // request after 0x1E43C/0x2F32 complete; the child conversion owns the
    // exact world-activity and scene-state writes.
    if (state.world_activity_requested) {
        if (!callbacks.handle_world_activity_request) {
            return FieldRuntimeFrameResult::world_activity_pending;
        }
        callbacks.handle_world_activity_request(state);
        state.world_activity_requested = false;
        return FieldRuntimeFrameResult::world_activity_dispatched;
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
