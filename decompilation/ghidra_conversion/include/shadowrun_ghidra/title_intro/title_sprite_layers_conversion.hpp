#pragma once

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct TitleSpriteLayerState {
    bool alternate_sources = false; // FFDF8C
    std::int8_t first_phase = 0;    // FF1580 +0x0D
    std::int8_t second_phase = 0;   // FF1580 +0x0C
    std::int8_t first_delay = 0;    // FF1580 +0x03
    std::int8_t second_delay = 0;   // FF1580 +0x02
    std::int8_t first_limit = 0;
    std::int8_t second_limit = 0;
    std::uint8_t record_flags = 0;  // FF1580 +0x12
    std::uint16_t profile_selector = 0; // FFDF88
    std::uint8_t effect_timer = 0;  // FFE169
    std::uint16_t effect_active = 0; // FFE084
    std::uint16_t base_x = 0;       // FF1580 +0x0E
    std::uint16_t base_y = 0;       // FF1580 +0x10
};

struct TitleSpriteLayerSources {
    std::uint32_t first_source_token = 0;
    std::uint32_t second_source_token = 0;
};

struct TitleSpriteLayerBuild {
    std::uint32_t source_token = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t build_flags = 0x6000U;
    bool status_overlay = false;
};

struct TitleSpriteLayerCallbacks {
    std::function<std::optional<std::uint32_t>(
        std::int8_t phase)>
        resolve_first_source;
    std::function<std::optional<std::uint32_t>(
        std::int8_t phase)>
        resolve_second_source;
    // ROM 0x1C5EC supplies one of four four-word status-overlay tables.
    std::function<std::optional<std::int16_t>(
        std::uint16_t effect_frame, std::uint16_t selector)>
        resolve_status_overlay_offset;
    std::function<void(const TitleSpriteLayerBuild&)> build_sprite;
    std::function<void()> build_title_sprites_twice;
};

enum class TitleSpriteLayerAction {
    updated,
    rendered,
    sprite_source_unresolved,
    overlay_unresolved,
};

// ROM 0x1C506: render the two selected title sources, optional status
// overlays, and the source's late double-build gate.
TitleSpriteLayerAction render_title_sprite_layers(
    TitleSpriteLayerState& state,
    const TitleSpriteLayerSources& sources,
    const TitleSpriteLayerCallbacks& callbacks);

// ROM 0x1C3FA: update the two source phases/delays, choose the alternate
// fixed source pair or the phase-selected pair, then call 0x1C506.
TitleSpriteLayerAction advance_title_sprite_layers(
    TitleSpriteLayerState& state,
    const TitleSpriteLayerCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
