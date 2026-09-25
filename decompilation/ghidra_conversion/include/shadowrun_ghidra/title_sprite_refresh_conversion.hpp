#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct TitleSpriteRefreshState {
    bool title_mode_visible = false; // FFF0D2
    bool records_pending = false; // FFE0EE
    std::int16_t refresh_countdown = 0; // FFE0F0
    std::uint16_t render_flags = 0; // FFDF88
};

struct TitleSpriteRefreshCallbacks {
    std::function<void()> build_title_sprite_records;
};

enum class TitleSpriteRefreshAction {
    title_mode_inactive,
    throttled,
    records_built,
};

// ROM 0x26972: refresh title sprite records unless a pending animation and
// its source countdown still defer the build for the current render phase.
TitleSpriteRefreshAction refresh_title_sprite_records(
    TitleSpriteRefreshState& state,
    const TitleSpriteRefreshCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
