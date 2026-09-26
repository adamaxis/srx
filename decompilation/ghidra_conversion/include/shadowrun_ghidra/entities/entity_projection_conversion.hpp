#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct EntityProjectionState {
    std::uint16_t world_x = 0;
    std::uint16_t world_y = 0;
    std::uint16_t sprite_offset_y = 0;
    std::uint16_t screen_x = 0;
    std::uint16_t screen_y = 0;
};

enum class EntityProjectionMode {
    StandardField,
    FullScreen,
};

struct EntityProjectionResult {
    std::uint16_t source_result = 0;
    bool visible = false;
    bool wrote_screen_x = false;
    bool wrote_screen_y = false;
};

// ROM 0x5A56: project an entity from fixed-point world coordinates into
// screen coordinates. The source's callback identity selects either the
// normal field window or the wider full-screen window; the enum is the typed
// host equivalent of that identity check.
EntityProjectionResult project_entity_screen_position(
    EntityProjectionState& state,
    std::uint16_t camera_x,
    std::uint16_t camera_y,
    EntityProjectionMode mode);

// ROM 0x5A62: project an ordinary field entity using the normal window.
EntityProjectionResult project_entity_screen_position_standard(
    EntityProjectionState& state,
    std::uint16_t camera_x,
    std::uint16_t camera_y);

} // namespace shadowrun::ghidra_conversion
