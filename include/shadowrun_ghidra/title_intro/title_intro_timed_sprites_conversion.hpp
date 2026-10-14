#pragma once

#include "shadowrun_ghidra/title_intro/title_intro_motion_conversion.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kTitleIntroTimedSpriteCount = 6;

struct TitleIntroTimedSpriteState {
    std::uint16_t current_frame = 0;
    std::uint16_t next_event_frame = 0;
    std::uint16_t descriptor_index = 0;
    std::uint32_t current_sprite_descriptor = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
};

struct TitleIntroTimedSpriteCallbacks {
    std::function<void(const TitleIntroPositionedSpriteRequest&)>
        draw_sprite;
};

enum class TitleIntroTimedSpriteAdvanceResult {
    no_op,
    sprite_rendered,
    sequence_finished,
};

// ROM 0x5A572: install the terminal six-frame state used before a new timed
// sprite sequence is selected.
void reset_title_intro_timed_sprites(TitleIntroTimedSpriteState& state);

// ROM 0x5A582: begin a six-descriptor sequence at the supplied source
// position. The descriptors are host asset handles.
void begin_title_intro_timed_sprites(TitleIntroTimedSpriteState& state,
                                      std::uint16_t x, std::uint16_t y);

// ROM 0x5A59C: select one descriptor every eight frames, draw the current
// descriptor on active calls, and stop once the terminal index is due.
TitleIntroTimedSpriteAdvanceResult advance_title_intro_timed_sprites(
    TitleIntroTimedSpriteState& state,
    const std::array<std::uint32_t, kTitleIntroTimedSpriteCount>& descriptors,
    const TitleIntroTimedSpriteCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
