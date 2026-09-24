#include "shadowrun_ghidra/field_scene_entry_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void initialize_field_scene_entry(
    FieldSceneEntryState& state,
    const FieldSceneEntryCallbacks& callbacks) {
    if (callbacks.prepare_display) {
        callbacks.prepare_display();
    }
    state.field_transition_marker = 0U;
    state.scene_refresh_marker = 0xFFFFU;
    state.pending_scene_marker = 0xFFFFU;
    if (callbacks.refresh_scene_profile) {
        callbacks.refresh_scene_profile();
    }
    if (callbacks.refresh_scene_state) {
        callbacks.refresh_scene_state();
    }
    if (callbacks.apply_scene_profile) {
        callbacks.apply_scene_profile();
    }
    if (callbacks.render_scene) {
        callbacks.render_scene();
    }
    if (callbacks.upload_scene_profile) {
        callbacks.upload_scene_profile();
    }
    if (callbacks.guard_world_effect) {
        callbacks.guard_world_effect();
    }
}

void reinitialize_field_scene_entry(
    FieldSceneEntryState& state,
    const FieldSceneEntryCallbacks& callbacks) {
    state.scene_refresh_marker = 0xFFFFU;
    if (callbacks.refresh_scene_state) {
        callbacks.refresh_scene_state();
    }
    if (callbacks.guard_world_effect) {
        callbacks.guard_world_effect();
    }
    if (callbacks.prepare_display) {
        callbacks.prepare_display();
    }
    state.camera_x = static_cast<std::uint16_t>(state.camera_x & 0xFF80U);
    state.camera_y = static_cast<std::uint16_t>(state.camera_y & 0xFF80U);
    state.camera_transition_x = 0xFFFFU;
    state.camera_transition_y = 0xFFFFU;
    if (callbacks.render_scene) {
        callbacks.render_scene();
    }
    if (callbacks.upload_scene_profile) {
        callbacks.upload_scene_profile();
    }
}

} // namespace shadowrun::ghidra_conversion
