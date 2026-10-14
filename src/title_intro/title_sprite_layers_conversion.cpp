#include "shadowrun_ghidra/title_intro/title_sprite_layers_conversion.hpp"

#include <array>

namespace shadowrun::ghidra_conversion {
namespace {

// ROM 0x1C5EC points at these four-word tables:
//   0x1C5FC: 003C 0000 0000 0000
//   0x1C604: 003C 00C4 0000 0000
//   0x1C60C: 003C 00C4 0078 0000
//   0x1C614: 003C 00C4 0078 0088
constexpr std::array<std::array<std::int16_t, 4>, 4>
    kTitleStatusOverlayOffsets{{
        {{0x003C, 0x0000, 0x0000, 0x0000}},
        {{0x003C, 0x00C4, 0x0000, 0x0000}},
        {{0x003C, 0x00C4, 0x0078, 0x0000}},
        {{0x003C, 0x00C4, 0x0078, 0x0088}},
    }};

bool decrement_byte_until_negative(std::int8_t& value) {
    value = static_cast<std::int8_t>(value - 1);
    return value < 0;
}

TitleSpriteLayerAction build_base_sprite(
    const std::uint32_t source_token, const std::uint16_t x,
    const std::uint16_t y, const TitleSpriteLayerCallbacks& callbacks) {
    if (callbacks.build_sprite) {
        callbacks.build_sprite(TitleSpriteLayerBuild{
            source_token, x, y, 0x6000U, false});
    }
    return TitleSpriteLayerAction::rendered;
}

} // namespace

std::optional<std::int16_t> resolve_title_status_overlay_offset(
    const std::uint16_t effect_frame, const std::uint16_t selector) {
    if (effect_frame >= kTitleStatusOverlayOffsets.size() ||
        selector >= kTitleStatusOverlayOffsets[effect_frame].size()) {
        return std::nullopt;
    }
    return kTitleStatusOverlayOffsets[effect_frame][selector];
}

TitleSpriteLayerAction render_title_sprite_layers(
    TitleSpriteLayerState& state,
    const TitleSpriteLayerSources& sources,
    const TitleSpriteLayerCallbacks& callbacks) {
    if (sources.first_source_token == 0U ||
        sources.second_source_token == 0U) {
        return TitleSpriteLayerAction::sprite_source_unresolved;
    }
    build_base_sprite(sources.first_source_token, state.base_x, state.base_y,
                      callbacks);

    std::optional<std::int16_t> overlay_offset;
    if ((state.record_flags & 0x02U) != 0U) {
        // 68000 source: move.b FFE169,D0; ext.w; subq.w #1,D0; clamp
        // positive values >=4 to 3; andi.w #3,D0. A zero timer therefore
        // selects table row 3 after the signed underflow, rather than row 0.
        auto effect_frame = static_cast<std::int16_t>(
                                static_cast<std::int8_t>(state.effect_timer)) -
                            1;
        if (effect_frame >= 4) {
            effect_frame = 3;
        }
        effect_frame = static_cast<std::int16_t>(effect_frame & 0x0003);
        overlay_offset = callbacks.resolve_status_overlay_offset
                             ? callbacks.resolve_status_overlay_offset(
                                   static_cast<std::uint16_t>(effect_frame),
                                   state.profile_selector & 3U)
                             : resolve_title_status_overlay_offset(
                                   static_cast<std::uint16_t>(effect_frame),
                                   state.profile_selector & 3U);
        if (!overlay_offset.has_value()) {
            return TitleSpriteLayerAction::overlay_unresolved;
        }
        if (*overlay_offset != 0 && callbacks.build_sprite) {
            callbacks.build_sprite(TitleSpriteLayerBuild{
                sources.first_source_token,
                static_cast<std::uint16_t>(state.base_x + *overlay_offset),
                state.base_y, 0x6000U, true});
        }
    }

    build_base_sprite(sources.second_source_token, state.base_x, state.base_y,
                      callbacks);
    if ((state.record_flags & 0x02U) != 0U && *overlay_offset != 0 &&
        callbacks.build_sprite) {
        callbacks.build_sprite(TitleSpriteLayerBuild{
            sources.second_source_token,
            static_cast<std::uint16_t>(state.base_x + *overlay_offset),
            state.base_y, 0x6000U, true});
    }
    if (state.effect_active != 0U &&
        (state.profile_selector & 0x000FU) >= 8U &&
        callbacks.build_title_sprites_twice) {
        callbacks.build_title_sprites_twice();
    }
    return TitleSpriteLayerAction::rendered;
}

TitleSpriteLayerAction advance_title_sprite_layers(
    TitleSpriteLayerState& state,
    const TitleSpriteLayerCallbacks& callbacks) {
    TitleSpriteLayerSources sources{};
    if (state.alternate_sources) {
        sources.first_source_token = 0x0001D428U;
        sources.second_source_token = 0x0001D452U;
    } else {
        if (state.second_phase < state.second_limit &&
            decrement_byte_until_negative(state.second_delay)) {
            state.second_delay = 4;
            const auto next = static_cast<std::int8_t>(state.second_phase + 1);
            if (next <= state.second_limit) {
                state.second_phase = next;
            }
        }
        if (state.first_phase < state.first_limit &&
            decrement_byte_until_negative(state.first_delay)) {
            state.first_delay = 4;
            const auto next = static_cast<std::int8_t>(state.first_phase + 1);
            if (next <= state.first_limit) {
                state.first_phase = next;
            }
        }
        const auto first = callbacks.resolve_first_source
                               ? callbacks.resolve_first_source(state.first_phase)
                               : std::nullopt;
        const auto second = callbacks.resolve_second_source
                                ? callbacks.resolve_second_source(
                                      state.second_phase)
                                : std::nullopt;
        if (!first.has_value() || !second.has_value()) {
            return TitleSpriteLayerAction::sprite_source_unresolved;
        }
        // A2 is selected from the primary (+0x04) descriptor and A4 from the
        // secondary (+0x08) descriptor. The previous conversion had these
        // assignments reversed, which swapped the two composed title layers.
        sources.first_source_token = *first;
        sources.second_source_token = *second;
    }
    return render_title_sprite_layers(state, sources, callbacks);
}

} // namespace shadowrun::ghidra_conversion
