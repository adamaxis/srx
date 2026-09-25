#include "shadowrun_ghidra/title_effect_sprites_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void advance_title_effect_sprites(
    TitleEffectSpritesState& state,
    const TitleEffectSpritesCallbacks& callbacks) {
    state.positions[2].x = static_cast<std::int16_t>(
        state.positions[2].x + 9);
    state.positions[3].x = static_cast<std::int16_t>(
        state.positions[3].x + 9);
    state.positions[4].x = static_cast<std::int16_t>(
        state.positions[4].x - 0x1A);
    state.positions[5].x = static_cast<std::int16_t>(
        state.positions[5].x - 0x1A);
    state.positions[0].x = static_cast<std::int16_t>(
        state.positions[0].x - 8);
    state.positions[1].x = static_cast<std::int16_t>(
        state.positions[1].x - 8);

    const auto upper_y_delta = static_cast<std::int16_t>(
        state.alternate_motion ? -0x18 : -0x36);
    const auto lower_y_delta = static_cast<std::int16_t>(
        state.alternate_motion ? -4 : 0x1A);
    for (auto& position : state.positions) {
        position.y = static_cast<std::int16_t>(position.y + upper_y_delta);
    }
    state.positions[1].y = static_cast<std::int16_t>(
        state.positions[1].y + lower_y_delta - upper_y_delta);
    state.positions[3].y = static_cast<std::int16_t>(
        state.positions[3].y + lower_y_delta - upper_y_delta);
    state.positions[5].y = static_cast<std::int16_t>(
        state.positions[5].y + lower_y_delta - upper_y_delta);

    if (!callbacks.build_sprite) {
        return;
    }
    for (const auto& position : state.positions) {
        callbacks.build_sprite(
            state.source_table_token, static_cast<std::uint16_t>(position.x),
            static_cast<std::uint16_t>(position.y), state.build_flags);
    }
}

} // namespace shadowrun::ghidra_conversion
