#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct FieldModeExitState {
    std::uint8_t scene_id = 0;
    std::uint8_t submode = 0;
    std::uint8_t menu_mode = 0; // FFF0B6
    std::int8_t alternate_result = 0; // FFF0D5
    std::uint32_t mode_flags = 0; // FFFC30
    std::uint32_t world_flags = 0; // FFFC1E
    std::uint32_t effect_flags = 0; // FFFC34
    std::uint32_t render_flags = 0; // FFFC38
    std::uint32_t service_flags = 0; // FFFC22
    std::uint16_t service_options = 0; // FFFC2A
    std::uint32_t effect_marker = 0; // FFDD64
};

struct FieldModeExitCallbacks {
    std::function<void(std::uint32_t marker)> mark_effect_bit;
    // The prompt/resource coordinator may change alternate_result while it
    // services the selected mode, just as the source coordinator changes
    // FFF0D5 before returning.
    std::function<void(FieldModeExitState&)> run_mode_exit_services;
    std::function<void()> refresh_mode_display;
};

enum class FieldModeExitAction {
    no_matching_route,
    services_completed,
    services_completed_and_display_refreshed,
};

// ROM 0x1F830: choose the mode-exit route, apply its guarded field-state
// mutations, clear the alternate-result marker before the service call, and
// refresh the mode display if the service returns a nonzero result marker.
FieldModeExitAction prepare_field_mode_exit(
    FieldModeExitState& state,
    const FieldModeExitCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
