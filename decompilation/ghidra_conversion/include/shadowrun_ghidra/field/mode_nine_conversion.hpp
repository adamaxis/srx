#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct ModeNineInitializationState {
    // These fields correspond to the source writes at 0x2A94 without
    // exposing the Genesis RAM addresses they occupied.
    bool mode_nine_resource_selected = false; // FFE114 = resource 0x2ADA
    std::uint8_t world_scene_mode = 0;        // FFDDC4
    std::uint16_t field_mode_service_state = 0; // FFDF40
    std::int16_t camera_transition_marker = 0;  // FFDF8E = -1
    std::int16_t field_mode_transition_index = 0; // FFFC10 = 3
    bool alternate_scene_profile_selected = false; // FFFC0E = 1
    bool scene_profile_override_lock = false;      // FFE10E = 1
};

struct ModeNineInitializationCallbacks {
    std::function<void()> prepare_mode_entry;
    // The direct source installs resource pointer 0x2ADA. The host must map
    // that source record to a verified direct asset alias.
    std::function<void()> select_mode_nine_resource;
};

// ROM 0x2A94: prepare the mode-9 field entry, install source resource 0x2ADA,
// publish world mode 9, clear the field-mode service word, set the camera
// transition marker to -1, select transition index 3, enable the alternate
// profile, and hold the scene-profile override lock.
void initialize_mode_nine(
    ModeNineInitializationState& state,
    const ModeNineInitializationCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
