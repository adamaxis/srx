#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct TitleIntroPositionedSpriteRequest {
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t palette_flags = 0;
};

struct TitleIntroMotionPhase {
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t delay_frames = 0;
    // The source only replaces the position when this table word is nonzero.
    std::uint16_t position_if_nonzero = 0;
    std::uint16_t position_step = 0;
};

constexpr std::size_t kTitleIntroMotionPhaseCount = 5;

struct TitleIntroMotionState {
    std::uint16_t current_frame = 0;
    std::uint16_t animation_start_frame = 0;
    std::uint16_t next_event_frame = 0;
    std::uint16_t phase = 0;
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t position = 0;
    std::uint16_t position_step = 0;
};

struct TitleIntroMotionCallbacks {
    std::function<void(const TitleIntroPositionedSpriteRequest&)>
        draw_motion_sprite;
};

enum class TitleIntroMotionAdvanceResult {
    no_op,
    sprite_rendered,
    sequence_finished,
};

// ROM 0x5A3B0: reset the timed motion sequence deadline and phase index.
void reset_title_intro_motion(TitleIntroMotionState& state);

// ROM 0x5A3BE: advance the five-entry motion table, including its source
// elapsed-frame cutoff, then project the current descriptor with the shared
// high-byte scroll offset on every active call.
TitleIntroMotionAdvanceResult advance_title_intro_motion(
    TitleIntroMotionState& state,
    const std::array<TitleIntroMotionPhase, kTitleIntroMotionPhaseCount>&
        phases,
    std::uint16_t scroll_accumulator,
    const TitleIntroMotionCallbacks& callbacks);

constexpr std::size_t kTitleIntroDualSpriteCount = 8;

struct TitleIntroDualSpriteState {
    std::uint16_t current_frame = 0;
    std::uint16_t next_event_frame = 0;
    std::uint16_t phase = 0;
    std::uint32_t left_sprite_descriptor = 0;
    std::uint32_t right_sprite_descriptor = 0;
};

struct TitleIntroDualSpriteCallbacks {
    std::function<void(const TitleIntroPositionedSpriteRequest&)>
        draw_left_sprite;
    std::function<void(const TitleIntroPositionedSpriteRequest&)>
        draw_right_sprite;
};

// ROM 0x5A4B4: reset the four-phase dual-sprite sequence.
void reset_title_intro_dual_sprites(TitleIntroDualSpriteState& state);

// ROM 0x5A4C2: select paired descriptors from the eight-entry host table,
// advance every six frames, and draw both sprites on each active call.
void advance_title_intro_dual_sprites(
    TitleIntroDualSpriteState& state,
    const std::array<std::uint32_t, kTitleIntroDualSpriteCount>& sprites,
    const TitleIntroDualSpriteCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
