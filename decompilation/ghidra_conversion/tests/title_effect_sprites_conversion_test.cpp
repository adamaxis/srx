#include "shadowrun_ghidra/title_effect_sprites_conversion.hpp"

#include <array>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleEffectSpritesState state{};
    state.positions = {{{100, 200}, {110, 210}, {120, 220},
                        {130, 230}, {140, 240}, {150, 250}}};
    state.source_table_token = 0x1D158U;
    state.build_flags = 0x2000U;
    std::array<TitleEffectSpritePosition, 6> emitted{};
    int builds = 0;
    TitleEffectSpritesCallbacks callbacks{};
    callbacks.build_sprite =
        [&](const std::uint32_t source, const std::uint16_t x,
            const std::uint16_t y, const std::uint16_t flags) {
        if (source != 0x1D158U || flags != 0x2000U || builds >= 6) {
            builds += 100;
            return;
        }
        emitted[builds] = {static_cast<std::int16_t>(x),
                           static_cast<std::int16_t>(y)};
        ++builds;
    };
    advance_title_effect_sprites(state, callbacks);
    const std::array<TitleEffectSpritePosition, 6> expected =
        {{{92, 146}, {102, 236}, {129, 166}, {139, 256},
          {114, 186}, {124, 276}}};
    if (builds != 6 || emitted != expected || state.positions != expected) {
        return 1;
    }

    state = {};
    state.positions = {{{100, 200}, {110, 210}, {120, 220},
                        {130, 230}, {140, 240}, {150, 250}}};
    state.alternate_motion = true;
    advance_title_effect_sprites(state, callbacks);
    const std::array<TitleEffectSpritePosition, 6> alternate_expected =
        {{{92, 176}, {102, 206}, {129, 196}, {139, 226},
          {114, 216}, {124, 246}}};
    if (state.positions != alternate_expected) {
        return 2;
    }
    return 0;
}
