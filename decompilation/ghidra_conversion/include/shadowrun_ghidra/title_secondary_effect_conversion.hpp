#pragma once

#include "shadowrun_ghidra/title_effect_sprites_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct TitleSecondaryEffectSpriteBuild {
    // The source pointer is provenance only. The host renderer consumes the
    // resolved asset token and never looks up a ROM address at runtime.
    std::uint32_t source_table_token = 0;
    std::uint32_t sprite_asset_token = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t build_flags = 0x2000U;
};

struct TitleSecondaryEffectState {
    bool active = false;                  // FF15CC
    std::uint8_t phase_delay = 0;         // FF15D0
    std::uint8_t animation_phase = 0;     // FF15CF
    std::uint8_t source_variant = 0;      // FF15CB
    std::uint8_t frame_delay = 0;         // FF15CA
    std::uint8_t lifetime = 0;            // FF15D1
    std::int16_t position_x = 0;          // FF15C6, fixed-point source unit
    std::int16_t position_y = 0;          // FF15C8, fixed-point source unit

    bool alternate_motion = false;        // FFE0A8 != 0
    std::uint16_t burst_countdown = 0;    // FFE0AA
    std::uint8_t title_service_flags = 0; // FF1592
    std::uint16_t transition_state = 0;   // FFE0D6

    // FFE088/8A/8C/8E: the paired title-effect positions used by 0x1CEFA.
    std::array<TitleEffectSpritePosition, 2> paired_positions{};
    // FFE090..FFE0A6: the six positions used by 0x1CF4A.
    TitleEffectSpritesState expanded_sprites{};
};

struct TitleSecondaryEffectCallbacks {
    // Resolve one of the nine source sprite sequences into a named host
    // asset. The ROM pointer is supplied only as extraction provenance.
    std::function<std::optional<std::uint32_t>(
        std::uint32_t source_table_token)>
        resolve_sprite_asset;
    std::function<void(const TitleSecondaryEffectSpriteBuild&)> build_sprite;
    std::function<void()> run_menu_progression_variant; // 0x19F18
};

enum class TitleSecondaryEffectAction {
    inactive,
    rendered,
    expired,
    sprite_asset_unresolved,
};

// ROM 0x1CD5A: advance the secondary title/world effect selected by
// FF1592 bit 0x40. This owns the effect's three timers, source-variant and
// phase selection, current sprite build, paired-position seeding, and the
// terminal handoff back to menu progression.
TitleSecondaryEffectAction advance_title_secondary_effect_service(
    TitleSecondaryEffectState& state,
    const TitleSecondaryEffectCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
