#pragma once

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

enum class FieldModeProfile {
    profile_a,
    profile_b,
};

struct FieldModeInitializationState {
    FieldModeProfile profile = FieldModeProfile::profile_a;
    std::array<std::uint8_t, 7> transient_fields{};
    bool render_pending = false;
};

struct FieldModeInitializationCallbacks {
    std::function<void(FieldModeProfile)> select_profile_data;
    std::function<void()> prepare_mode_entry;
    std::function<void()> initialize_scene_mode_state;
};

// ROM 0x2A32 / 0x2A44: select one of two mode profiles, run the common mode
// entry service, clear the seven transient state bytes and render-pending
// marker, then initialize scene-mode state. Profile tables stay a host-owned
// data boundary rather than becoming ROM-addressed runtime state.
void initialize_field_mode(
    FieldModeInitializationState& state,
    FieldModeProfile profile,
    const FieldModeInitializationCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
