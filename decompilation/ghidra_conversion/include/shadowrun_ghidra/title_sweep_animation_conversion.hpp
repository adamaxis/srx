#pragma once

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct TitleSweepAnimationState {
    std::uint8_t record_flags = 0;     // FF1580 +0x12
    std::uint8_t phase = 0;             // FF1580 +0x13
    std::int8_t frame_delay = 0;        // FF1580 +0x14
    std::uint16_t base_x = 0;           // FF1580 +0x0E
    std::uint16_t base_y = 0;           // FF1580 +0x10
    std::uint16_t transition_state = 0; // FFE0D6
};

struct TitleSweepFrame {
    std::uint32_t source_table_token = 0x0001C6BCU;
    std::int16_t horizontal_offset = 0;
    std::uint8_t next_delay = 0;
    std::uint32_t sprite_data_token = 0;
};

struct TitleSweepSpriteBuild {
    std::uint32_t source_table_token = 0x0001C6BCU;
    std::uint16_t phase = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t build_flags = 0x6000U;
    std::uint32_t sprite_data_token = 0;
};

struct TitleSweepAnimationCallbacks {
    // ROM 0x1C6D4 selects a pointer-table entry at 0x1C6BC, a signed x
    // offset, and the next byte delay for the 32-phase sweep.
    std::function<std::optional<TitleSweepFrame>(
        std::uint16_t phase)>
        resolve_frame;
    std::function<void(const TitleSweepSpriteBuild&)> build_sprite;
    std::function<void()> attach_behavior_descriptor_b;
    std::function<void()> return_to_title_state;
};

enum class TitleSweepAnimationAction {
    waiting,
    rendered,
    finalized,
    frame_unresolved,
};

// ROM 0x1C61C: advance the delayed 32-phase sweep. Each active phase emits
// the same source sprite at base_x +/- signed offset; phase 0x20 clears the
// source record flags and returns through the title-state boundary.
TitleSweepAnimationAction advance_title_sweep_animation(
    TitleSweepAnimationState& state,
    const TitleSweepAnimationCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
