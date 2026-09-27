#include "shadowrun_ghidra/field/field_scene_entry_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void initialize_field_scene_entry(
    FieldSceneEntryState& state,
    const FieldSceneEntryCallbacks& callbacks) {
    if (callbacks.prepare_field_resources) {
        callbacks.prepare_field_resources();
    }
    state.field_transition_marker = 0U;
    state.scene_refresh_marker = 0xFFFFU;
    state.pending_scene_marker = 0xFFFFU;
    if (callbacks.prepare_field_scene_buffers) {
        callbacks.prepare_field_scene_buffers();
    }
    if (callbacks.run_world_frame_services) {
        callbacks.run_world_frame_services();
    }
    if (callbacks.apply_scene_profile_selection) {
        callbacks.apply_scene_profile_selection();
    }
    if (callbacks.prepare_field_viewports) {
        callbacks.prepare_field_viewports();
    }
    if (callbacks.upload_scene_profile_block) {
        callbacks.upload_scene_profile_block();
    }
    if (callbacks.guard_world_effect_player_spawn) {
        callbacks.guard_world_effect_player_spawn();
    }
}

void reinitialize_field_scene_entry(
    FieldSceneEntryState& state,
    const FieldSceneEntryCallbacks& callbacks) {
    state.scene_refresh_marker = 0xFFFFU;
    if (callbacks.run_world_frame_services) {
        callbacks.run_world_frame_services();
    }
    if (callbacks.guard_world_effect_player_spawn) {
        callbacks.guard_world_effect_player_spawn();
    }
    if (callbacks.prepare_field_resources) {
        callbacks.prepare_field_resources();
    }
    state.camera_x = static_cast<std::uint16_t>(state.camera_x & 0xFF80U);
    state.camera_y = static_cast<std::uint16_t>(state.camera_y & 0xFF80U);
    state.camera_transition_x = 0xFFFFU;
    state.camera_transition_y = 0xFFFFU;
    if (callbacks.prepare_field_viewports) {
        callbacks.prepare_field_viewports();
    }
    if (callbacks.upload_scene_profile_block) {
        callbacks.upload_scene_profile_block();
    }
}

void advance_field_scene_transition(
    FieldSceneTransitionState& state,
    std::vector<std::uint8_t>& event_counters,
    const FieldSceneEntryCallbacks& callbacks) {
    if (callbacks.step_palette_transition) {
        callbacks.step_palette_transition();
    }

    if (state.field_transition_marker == 0U) {
        state.camera_x = static_cast<std::uint16_t>(state.camera_x & 0xFF80U);
        state.camera_y = static_cast<std::uint16_t>(state.camera_y & 0xFF80U);
        state.camera_transition_x = 0xFFFFU;
        state.camera_transition_y = 0xFFFFU;
        if (callbacks.run_world_frame_services) {
            callbacks.run_world_frame_services();
        }
        if (callbacks.guard_world_effect_player_spawn) {
            callbacks.guard_world_effect_player_spawn();
        }
        if (state.pending_event_index != 0U) {
            const auto event_index = state.pending_event_index;
            state.pending_event_index = 0U;
            if (state.event_state_negative && event_index < event_counters.size()) {
                event_counters[event_index] = static_cast<std::uint8_t>(
                    (event_counters[event_index] & 0xE0U) + 1U);
                if (callbacks.record_pending_event) {
                    callbacks.record_pending_event(event_index);
                }
            }
        }
        if (callbacks.prepare_field_viewports) {
            callbacks.prepare_field_viewports();
        }
        if (callbacks.upload_scene_profile_block) {
            callbacks.upload_scene_profile_block();
        }
        return;
    }

    state.field_transition_marker = 0U;
    state.scene_refresh_marker = 0xFFFFU;
    state.pending_scene_marker = 0xFFFFU;
    if (callbacks.prepare_field_scene_buffers) {
        callbacks.prepare_field_scene_buffers();
    }
    if (callbacks.run_world_frame_services) {
        callbacks.run_world_frame_services();
    }
    if (callbacks.apply_scene_profile_selection) {
        callbacks.apply_scene_profile_selection();
    }
    if (callbacks.prepare_field_viewports) {
        callbacks.prepare_field_viewports();
    }
    if (callbacks.upload_scene_profile_block) {
        callbacks.upload_scene_profile_block();
    }
    if (callbacks.guard_world_effect_player_spawn) {
        callbacks.guard_world_effect_player_spawn();
    }
}

} // namespace shadowrun::ghidra_conversion
