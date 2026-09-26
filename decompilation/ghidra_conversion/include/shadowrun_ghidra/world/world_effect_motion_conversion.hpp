#pragma once

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct WorldEffectMotionState {
    std::uint16_t frame_countdown = 0;  // FFE070
    std::uint16_t motion_phase = 0;     // FFE072
    std::uint16_t position_x = 0;       // FFE06A
    std::uint16_t position_y = 0;       // FFE068
    std::int16_t horizontal_offset = 0; // FFE06C
    std::uint16_t completion_marker = 0; // FFE06E
    std::uint16_t transition_state = 0; // FFE0D6
};

struct WorldEffectMotionCallbacks {
    // ROM 0x1CA8C selects one of the source pointer-table entries at 0x1CAAC;
    // nonnegative horizontal motion adds five to that source index.
    std::function<std::optional<std::uint32_t>(
        std::uint16_t phase, bool reverse_table)>
        resolve_sprite_data;
    std::function<void(std::uint32_t sprite_data_token,
                       std::uint16_t x, std::uint16_t y,
                       std::uint16_t build_flags)>
        build_sprite;
    std::function<void()> run_menu_selection_service;
};

enum class WorldEffectMotionAction {
    waiting,
    rendered,
    finalized,
    sprite_data_unresolved,
};

// ROM 0x1CA12: advance the direction-dependent world-effect motion, subtract
// the source y offset, build the selected sprite, and finalize through the
// menu service when the countdown expires.
WorldEffectMotionAction advance_world_effect_motion(
    WorldEffectMotionState& state,
    const WorldEffectMotionCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
