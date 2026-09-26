#include "shadowrun_ghidra/title_intro/title_sprite_layers_conversion.hpp"

#include <optional>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleSpriteLayerState state{};
    state.first_limit = 2;
    state.second_limit = 2;
    state.first_delay = 0;
    state.second_delay = 0;
    state.base_x = 0x100U;
    state.base_y = 0x200U;
    std::uint32_t first = 0;
    std::uint32_t second = 0;
    int builds = 0;
    TitleSpriteLayerCallbacks callbacks{};
    callbacks.resolve_first_source =
        [&](const std::int8_t phase) -> std::optional<std::uint32_t> {
        first = static_cast<std::uint32_t>(phase) + 0x1000U;
        return first;
    };
    callbacks.resolve_second_source =
        [&](const std::int8_t phase) -> std::optional<std::uint32_t> {
        second = static_cast<std::uint32_t>(phase) + 0x2000U;
        return second;
    };
    callbacks.resolve_status_overlay_offset =
        [](const std::uint16_t, const std::uint16_t) {
            return std::optional<std::int16_t>(4);
        };
    callbacks.build_sprite = [&](const TitleSpriteLayerBuild& sprite) {
        if (sprite.build_flags != 0x6000U || sprite.y != 0x200U ||
            (sprite.x != 0x100U && sprite.x != 0x104U)) {
            builds += 100;
        }
        ++builds;
    };
    if (advance_title_sprite_layers(state, callbacks) !=
            TitleSpriteLayerAction::rendered ||
        state.first_phase != 1 || state.second_phase != 1 || builds != 2 ||
        first != 0x1001U || second != 0x2001U) {
        return 1;
    }

    state.record_flags = 0x02U;
    state.effect_timer = 8U;
    state.effect_active = 1U;
    state.profile_selector = 8U;
    callbacks.build_title_sprites_twice = [&] { builds += 10; };
    if (render_title_sprite_layers(
            state, TitleSpriteLayerSources{0x10U, 0x20U}, callbacks) !=
            TitleSpriteLayerAction::rendered ||
        builds != 16) {
        return 2;
    }

    TitleSpriteLayerState alternate{};
    alternate.alternate_sources = true;
    alternate.base_x = 0x80U;
    alternate.base_y = 0x90U;
    callbacks = {};
    callbacks.build_sprite = [&](const TitleSpriteLayerBuild& sprite) {
        if (sprite.source_token != 0x1D428U &&
            sprite.source_token != 0x1D452U) {
            builds += 100;
        }
        ++builds;
    };
    builds = 0;
    if (advance_title_sprite_layers(alternate, callbacks) !=
            TitleSpriteLayerAction::rendered || builds != 2) {
        return 3;
    }
    return 0;
}
