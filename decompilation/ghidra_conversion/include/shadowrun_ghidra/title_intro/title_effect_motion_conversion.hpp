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
    std::uint32_t first_source_token = 0;
    std::uint32_t second_source_token = 0;
    std::uint8_t first_phase = 0;
    std::uint8_t second_phase = 0;
};

struct TitleEffectMotionCallbacks {
    // ROM 0x1C926/0x1C956 selection depends on FFE0E2, FFE060, and frame.
    std::function<std::optional<std::uint32_t>(
        std::uint16_t effect_mode, std::uint16_t frame,
        std::uint16_t position_x)>
        resolve_sprite_data;
    std::function<void(std::uint32_t source_token,
                       std::uint8_t phase)>
        publish_source_record;
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

// ROM 0x1C866: advance the twelve-frame title effect at four ticks per frame,
// select the position/mode-dependent source table, and on completion install
// the corresponding 0x1C288/0x1C28E source record and clear menu state.
TitleEffectMotionAction advance_title_effect_motion(
    TitleEffectMotionState& state,
    const TitleEffectMotionCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
