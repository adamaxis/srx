#include "shadowrun_ghidra/world/world_frame_logic_conversion.hpp"
#include "enums.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

} // namespace

void advance_world_frame_logic(WorldFrameLogicState& state,
                               const WorldFrameLogicCallbacks& callbacks) {
    state.activity_started = false;
    state.target_selection_requested = false;
    state.service_records_reset = false;

    if (state.scene_state == 2U && state.transition_marker < 0 &&
        state.region_or_mode <= 9) {
        state.transition_marker = 0;
        state.scene_state = 3U;
    }

    if (state.activity_countdown != 0U) {
        --state.activity_countdown;
        if (state.activity_countdown == 0U) {
            call_once(callbacks.prepare_mode_entry);
            state.activity_handler_token = 0x0000C64CU;
            state.resource_countdown = 0x003CU;
            state.activity_started = true;
            call_once(callbacks.begin_world_activity);
        }
    }

    if (state.resource_countdown != 0U) {
        --state.resource_countdown;
    }

    if (state.world_active != 0U) {
        if (state.scene_state == 0U && state.event_lock == 0U &&
            state.world_mode != 0x0014U) {
            --state.cleanup_countdown;
            if (state.cleanup_countdown < 0) {
                state.service_records_reset = true;
                call_once(callbacks.reset_entity_service_records);
            }
        } else {
            const bool target_missing = !state.current_target_present ||
                                        !state.current_target_active;
            const bool special_target_gate =
                !state.current_entity_special ||
                state.current_entity_mode != 0x0CU;
            if (target_missing ||
                (special_target_gate && state.current_target_effect_locked)) {
                state.target_selection_requested = true;
                call_once(callbacks.select_entity_service_target);
            }
        }
    }

    call_once(callbacks.clear_inactive_entity_effects);
    // ROM 0x1DE7C gates the terrain batch and alarm countdown on FFDF40 == 2;
    // world_mode remains FFDDC4, the scene ID used by the cleanup branch. [C]
    if (state.field_mode ==
        static_cast<std::uint16_t>(srx::SceneMode::BUILDING)) {
        call_once(callbacks.process_terrain_work_batch);
        call_once(callbacks.advance_world_effect_trigger);
    }
    call_once(callbacks.trigger_world_effect_sequence);
    call_once(callbacks.resolve_nearby_effects);
}

} // namespace shadowrun::ghidra_conversion
