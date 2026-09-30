#include "shadowrun_ghidra/title_intro/title_secondary_effect_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

bool decrement_byte_until_negative(std::uint8_t& value) {
    value = static_cast<std::uint8_t>(value - 1U);
    return (value & 0x80U) != 0U;
}

} // namespace

std::optional<std::uint32_t> resolve_title_secondary_effect_source_table(
    const std::uint8_t variant, const std::uint8_t phase) {
    if (phase >= 3U) {
        return std::nullopt;
    }
    constexpr std::uint32_t kSources[3][3] = {
        {0x0001D906U, 0x0001D918U, 0x0001D922U},
        {0x0001D934U, 0x0001D93EU, 0x0001D948U},
        {0x0001D952U, 0x0001D95CU, 0x0001D966U},
    };
    const auto source_group = variant == 0U ? 0U : variant == 1U ? 1U : 2U;
    return kSources[source_group][phase];
}

namespace {

std::uint16_t to_screen_coordinate(const std::int16_t fixed_point) {
    return static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(fixed_point) >> 4U);
}

bool build_sprite(const std::uint32_t source_table_token,
                  const std::uint32_t sprite_asset_token,
                  const std::int16_t x, const std::int16_t y,
                  const TitleSecondaryEffectCallbacks& callbacks) {
    if (!callbacks.build_sprite) {
        return true;
    }
    callbacks.build_sprite(TitleSecondaryEffectSpriteBuild{
        source_table_token, sprite_asset_token, to_screen_coordinate(x),
        to_screen_coordinate(y), 0x2000U});
    return true;
}

void advance_paired_positions(TitleSecondaryEffectState& state,
                              const std::uint32_t source_table_token,
                              const std::uint32_t sprite_asset_token,
                              const TitleSecondaryEffectCallbacks& callbacks) {
    auto& first = state.paired_positions[0];
    auto& second = state.paired_positions[1];
    first.x = static_cast<std::int16_t>(first.x + 9);
    first.y = static_cast<std::int16_t>(first.y - 0x1A);
    second.x = static_cast<std::int16_t>(second.x - 0x0E);
    second.y = static_cast<std::int16_t>(second.y - 0x0E);
    build_sprite(source_table_token, sprite_asset_token, first.x, first.y,
                 callbacks);
    build_sprite(source_table_token, sprite_asset_token, second.x, second.y,
                 callbacks);
}

void seed_paired_positions_from_current(TitleSecondaryEffectState& state) {
    state.paired_positions[0] = {state.position_x, state.position_y};
    state.paired_positions[1] = {state.position_x, state.position_y};
}

void seed_expanded_positions_from_pairs(TitleSecondaryEffectState& state) {
    auto& positions = state.expanded_sprites.positions;
    positions[0] = {state.position_x, state.position_y};
    positions[1] = {state.position_x, state.position_y};
    positions[2] = state.paired_positions[0];
    positions[3] = state.paired_positions[0];
    positions[4] = state.paired_positions[1];
    positions[5] = state.paired_positions[1];
}

} // namespace

TitleSecondaryEffectAction advance_title_secondary_effect_service(
    TitleSecondaryEffectState& state,
    const TitleSecondaryEffectCallbacks& callbacks) {
    if (!state.active) {
        return TitleSecondaryEffectAction::inactive;
    }

    if (decrement_byte_until_negative(state.lifetime)) {
        state.active = false;
        if (state.alternate_motion) {
            state.burst_countdown = 0x0AU;
        }
    }

    if (decrement_byte_until_negative(state.frame_delay)) {
        state.frame_delay = 0x14U;
        if (state.source_variant < 2U) {
            ++state.source_variant;
        }
    }

    if (decrement_byte_until_negative(state.phase_delay)) {
        state.phase_delay = 2U;
        ++state.animation_phase;
        if (state.animation_phase >= 3U) {
            state.animation_phase = 0U;
        }
    }

    // ROM 0x1CDFE/0x1CE04 moves the fixed-point record before the first
    // build. The host conversion must publish those post-motion coordinates,
    // not the coordinates from the previous tick.
    state.position_x = static_cast<std::int16_t>(state.position_x - 0x0008);
    state.position_y = static_cast<std::int16_t>(state.position_y - 0x000E);

    const auto source_table = resolve_title_secondary_effect_source_table(
        state.source_variant, state.animation_phase);
    if (!source_table.has_value() || !callbacks.resolve_sprite_asset) {
        return TitleSecondaryEffectAction::sprite_asset_unresolved;
    }
    const auto sprite_asset =
        callbacks.resolve_sprite_asset(*source_table);
    if (!sprite_asset.has_value()) {
        return TitleSecondaryEffectAction::sprite_asset_unresolved;
    }

    build_sprite(*source_table, *sprite_asset, state.position_x,
                 state.position_y, callbacks);

    if (!state.active) {
        state.title_service_flags = static_cast<std::uint8_t>(
            state.title_service_flags & ~0x40U);
        state.transition_state = 0U;
        if (callbacks.run_menu_progression_variant) {
            callbacks.run_menu_progression_variant();
        }
        return TitleSecondaryEffectAction::expired;
    }

    if (state.frame_delay == 0U) {
        if (state.source_variant == 0U) {
            seed_paired_positions_from_current(state);
        } else if (state.source_variant == 1U) {
            seed_expanded_positions_from_pairs(state);
        }
    }

    if (state.source_variant > 1U) {
        state.expanded_sprites.alternate_motion = state.alternate_motion;
        state.expanded_sprites.source_table_token = *source_table;
        state.expanded_sprites.build_flags = 0x2000U;
        advance_title_effect_sprites(
            state.expanded_sprites,
            TitleEffectSpritesCallbacks{
                [&](const std::uint32_t, const std::uint16_t x,
                    const std::uint16_t y, const std::uint16_t flags) {
                    build_sprite(*source_table, *sprite_asset,
                                 static_cast<std::int16_t>(x),
                                 static_cast<std::int16_t>(y), callbacks);
                    (void)flags;
                }});
        advance_paired_positions(state, *source_table, *sprite_asset,
                                 callbacks);
    }
    return TitleSecondaryEffectAction::rendered;
}

} // namespace shadowrun::ghidra_conversion
