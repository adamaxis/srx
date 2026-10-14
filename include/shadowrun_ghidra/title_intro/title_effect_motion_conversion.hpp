#pragma once

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct TitleEffectMotionState {
    std::uint16_t frame_delay = 0;       // FFE066
    std::uint16_t frame = 0;             // FFE064
    std::uint16_t position_x = 0;        // FFE060
    std::uint16_t position_y = 0;        // FFE062
    std::uint16_t effect_mode = 0;       // FFE0E2
    std::uint16_t transition_state = 0;  // FFE0D6
    std::uint16_t menu_state = 0;        // FFE05E
    // Completion writes the secondary record on the left-side path and the
    // primary record on the right-side path, matching FF1588/FF158C and
    // FF1584/FF158D respectively.
    std::uint32_t secondary_behavior_descriptor = 0; // FF1588 (+8)
    std::uint32_t primary_behavior_descriptor = 0;   // FF1584 (+4)
    std::uint8_t secondary_stage = 0;                // FF1580 +0x0C
    std::uint8_t primary_stage = 0;                  // FF1580 +0x0D
};

enum class TitleEffectMotionRecordSlot {
    secondary,
    primary,
};

struct TitleEffectMotionCallbacks {
    // ROM 0x1C926/0x1C956 selection depends on FFE0E2, FFE060, and frame.
    std::function<std::optional<std::uint32_t>(
        std::uint16_t effect_mode, std::uint16_t frame,
        std::uint16_t position_x)>
        resolve_sprite_data;
    std::function<void(TitleEffectMotionRecordSlot slot,
                       std::uint32_t behavior_descriptor)>
        publish_behavior_descriptor;
    std::function<void(std::uint32_t sprite_data_token,
                       std::uint16_t x, std::uint16_t y,
                       std::uint16_t build_flags)>
        build_sprite;
    std::function<void()> run_menu_selection_service;
};

enum class TitleEffectMotionAction {
    waiting,
    rendered,
    finalized,
    sprite_data_unresolved,
};

// Typed extraction of the twelve direct sprite streams in each ROM table at
// 0x1C926 and 0x1C956. The source selects 0x1C956 when the nonzero effect-mode
// state agrees with the position being on the right side of 0x120; otherwise
// it selects 0x1C926.
std::optional<std::uint32_t> resolve_title_effect_motion_sprite_data(
    std::uint16_t effect_mode, std::uint16_t frame,
    std::uint16_t position_x);

// ROM 0x1C866: advance the twelve-frame title effect at four ticks per frame,
// select the position/mode-dependent source table, and on completion install
// the corresponding 0x1C288/0x1C28E source record and clear menu state.
TitleEffectMotionAction advance_title_effect_motion(
    TitleEffectMotionState& state,
    const TitleEffectMotionCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
