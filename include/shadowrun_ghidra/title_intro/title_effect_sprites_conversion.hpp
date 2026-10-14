#pragma once

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct TitleEffectSpritePosition {
    std::int16_t x = 0;
    std::int16_t y = 0;

    friend constexpr bool operator==(const TitleEffectSpritePosition& left,
                                     const TitleEffectSpritePosition& right) {
        return left.x == right.x && left.y == right.y;
    }

    friend constexpr bool operator!=(const TitleEffectSpritePosition& left,
                                     const TitleEffectSpritePosition& right) {
        return !(left == right);
    }
};

struct TitleEffectSpritesState {
    std::array<TitleEffectSpritePosition, 6> positions{};
    bool alternate_motion = false; // FFE0A8 != 0
    std::uint32_t source_table_token = 0;
    std::uint16_t build_flags = 0;
};

struct TitleEffectSpritesCallbacks {
    // ROM 0x1CF36 consumes the six positions with the source pointer and
    // attributes already held by the caller.
    std::function<void(std::uint32_t source_table_token,
                       std::uint16_t x, std::uint16_t y,
                       std::uint16_t build_flags)>
        build_sprite;
};

// ROM 0x1CF4A: advance the six paired title-effect positions and submit them
// in source order through the sprite-record builder boundary.
void advance_title_effect_sprites(TitleEffectSpritesState& state,
                                  const TitleEffectSpritesCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
