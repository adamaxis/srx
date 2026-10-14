#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct TitleIntroFinalSpriteRequest {
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t palette_flags = 0;
};

struct TitleIntroFinalProjectionPhase {
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t delay_frames = 0;
};

constexpr std::size_t kTitleIntroFinalProjectionPhaseCount = 3;

struct TitleIntroFinalProjectionState {
    std::uint16_t current_frame = 0;
    std::uint16_t animation_start_frame = 0;
    std::uint16_t next_event_frame = 0;
    std::uint16_t phase = 0;
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t x = 0;
};

// ROM 0x5A8C8: reset the late-intro projection deadline, phase, and x origin.
void reset_title_intro_final_projection(TitleIntroFinalProjectionState& state);

// ROM 0x5A8DE: gate the first phase on six elapsed frames, then advance three
// descriptors while continuing the four-word-per-call x projection between
// deadlines.
void advance_title_intro_final_projection(
    TitleIntroFinalProjectionState& state,
    const std::array<TitleIntroFinalProjectionPhase,
                     kTitleIntroFinalProjectionPhaseCount>& phases,
    const std::function<void(const TitleIntroFinalSpriteRequest&)>& draw_sprite);

struct TitleIntroTextRenderRequest {
    std::uint32_t text_asset = 0;
    std::uint16_t destination_word = 0;
    std::uint16_t palette_flags = 0;
};

constexpr std::size_t kTitleIntroTextBatchCount = 6;

// ROM 0x5A96A: submit the six final-intro text records in source order. Text
// assets are host handles; the original source stream pointers do not escape
// the extraction boundary.
void render_title_intro_text_batch(
    const std::array<std::uint32_t, kTitleIntroTextBatchCount>& text_assets,
    const std::function<void(const TitleIntroTextRenderRequest&)>& render_text);

} // namespace shadowrun::ghidra_conversion
