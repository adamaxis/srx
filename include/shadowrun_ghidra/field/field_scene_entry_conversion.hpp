#pragma once

#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct FieldSceneEntryState {
    std::uint16_t field_transition_marker = 0;
    std::uint16_t scene_refresh_marker = 0;
    std::uint16_t pending_scene_marker = 0;
    std::uint16_t camera_x = 0;
    std::uint16_t camera_y = 0;
    std::uint16_t camera_transition_x = 0;
    std::uint16_t camera_transition_y = 0;
};

struct FieldSceneEntryCallbacks {
    std::function<void()> prepare_field_resources;
    std::function<void()> prepare_field_scene_buffers;
    std::function<void()> run_world_frame_services;
    std::function<void()> apply_scene_profile_selection;
    std::function<void()> prepare_field_viewports;
    std::function<void()> upload_scene_profile_block;
    std::function<void()> guard_world_effect_player_spawn;
    std::function<void()> step_palette_transition;
    std::function<void(std::uint16_t)> record_pending_event;
};

struct FieldSceneTransitionState {
    std::uint16_t field_transition_marker = 0;
    std::uint16_t scene_refresh_marker = 0;
    std::uint16_t pending_scene_marker = 0;
    std::uint16_t camera_x = 0;
    std::uint16_t camera_y = 0;
    std::uint16_t camera_transition_x = 0;
    std::uint16_t camera_transition_y = 0;
    std::uint16_t pending_event_index = 0;
    bool event_state_negative = false;
};

// ROM 0x2BD2: step the palette transition, then test the field-transition
// marker. A nonzero marker re-enters the 0x281E field-resource/profile setup;
// the ordinary path aligns the camera, runs world-frame services, consumes a
// pending event marker, and refreshes/uploads the field view.
void advance_field_scene_transition(
    FieldSceneTransitionState& state,
    std::vector<std::uint8_t>& event_counters,
    const FieldSceneEntryCallbacks& callbacks);

// ROM 0x281E: initialize the field scene, reset its source markers, refresh
// the scene/profile state, render/upload the profile, then guard world-effect
// spawning. The individual services remain explicit host boundaries.
void initialize_field_scene_entry(
    FieldSceneEntryState& state,
    const FieldSceneEntryCallbacks& callbacks);

// ROM 0x28C2: reinitialize the scene through the alternate source order,
// align the camera coordinates to 0x80, reset camera-transition markers, and
// render/upload the resulting profile.
void reinitialize_field_scene_entry(
    FieldSceneEntryState& state,
    const FieldSceneEntryCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
