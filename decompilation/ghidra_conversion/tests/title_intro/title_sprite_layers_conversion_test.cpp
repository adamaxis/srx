#include "shadowrun_ghidra/title_intro/title_sprite_layers_conversion.hpp"

#include <array>
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
    std::uint32_t rendered_first = 0;
    std::uint32_t rendered_second = 0;
    int base_builds = 0;
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
        if (!sprite.status_overlay) {
            if (base_builds == 0) {
                rendered_first = sprite.source_token;
            } else if (base_builds == 1) {
                rendered_second = sprite.source_token;
            }
            ++base_builds;
        }
        if (sprite.build_flags != 0x6000U || sprite.y != 0x200U ||
            (sprite.x != 0x100U && sprite.x != 0x104U)) {
            builds += 100;
        }
        ++builds;
    };
    if (advance_title_sprite_layers(state, callbacks) !=
            TitleSpriteLayerAction::rendered ||
        state.first_phase != 1 || state.second_phase != 1 || builds != 2 ||
        first != 0x1001U || second != 0x2001U ||
        rendered_first != 0x1001U || rendered_second != 0x2001U) {
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

    if (resolve_title_status_overlay_offset(0U, 0U) != 0x003C ||
        resolve_title_status_overlay_offset(1U, 1U) != 0x00C4 ||
        resolve_title_status_overlay_offset(2U, 2U) != 0x0078 ||
        resolve_title_status_overlay_offset(3U, 3U) != 0x0088 ||
        resolve_title_status_overlay_offset(4U, 0U).has_value()) {
        return 3;
    }

    // The source subtracts before clamping: a zero FFE169 timer wraps to
    // signed -1 and then selects the final overlay table through &3.
    std::uint16_t observed_effect_frame = 0xFFFFU;
    std::uint16_t observed_selector = 0xFFFFU;
    callbacks.resolve_status_overlay_offset =
        [&](const std::uint16_t effect_frame, const std::uint16_t selector) {
            observed_effect_frame = effect_frame;
            observed_selector = selector;
            return std::optional<std::int16_t>(0);
        };
    state.effect_timer = 0U;
    state.effect_active = 0U;
    state.profile_selector = 1U;
    if (render_title_sprite_layers(
            state, TitleSpriteLayerSources{0x10U, 0x20U}, callbacks) !=
            TitleSpriteLayerAction::rendered ||
        observed_effect_frame != 3U || observed_selector != 1U) {
        return 4;
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
        return 5;
    }
    return 0;
}
