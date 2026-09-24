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
    std::function<void()> prepare_display;
    std::function<void()> refresh_scene_profile;
    std::function<void()> refresh_scene_state;
    std::function<void()> apply_scene_profile;
    std::function<void()> render_scene;
    std::function<void()> upload_scene_profile;
    std::function<void()> guard_world_effect;
};

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
