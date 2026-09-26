#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct ModeNineInitializationState {
    // These fields correspond to the source writes at 0x2A94 without
    // exposing the Genesis RAM addresses they occupied.
    std::uint8_t mode = 0;
    bool pending_resource = false;
    std::uint16_t field_marker = 0;
    std::uint8_t hud_mode = 0;
    std::uint8_t input_mode = 0;
    bool scene_state_pending = false;
};

struct ModeNineInitializationCallbacks {
    std::function<void()> prepare_mode_entry;
    std::function<void()> select_mode_resource;
};

// ROM 0x2A94: prepare the mode-9 field entry, publish its pending resource,
// reset the field marker, and establish the source mode/input flags.
void initialize_mode_nine(
    ModeNineInitializationState& state,
    const ModeNineInitializationCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
