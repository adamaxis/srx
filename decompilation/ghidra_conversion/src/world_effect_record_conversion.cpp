#include "shadowrun_ghidra/world_effect_record_conversion.hpp"

namespace shadowrun::ghidra_conversion {

WorldEffectRecordAction advance_world_effect_record(
    WorldEffectRecordState& state,
    const WorldEffectRecordCallbacks& callbacks) {
    state.lifetime = static_cast<std::uint8_t>(state.lifetime - 1U);
    if ((state.lifetime & 0x80U) != 0U) {
        state.active = 0U;
    }

    state.frame_delay = static_cast<std::uint8_t>(state.frame_delay - 1U);
    if ((state.frame_delay & 0x80U) != 0U) {
        state.frame_delay = 0x14U;
        if (state.source_variant < 2U) {
            ++state.source_variant;
        }
    }

    const auto easing_index = static_cast<std::uint8_t>(
        (0x3CU - state.lifetime) & 0x3FU);
    const auto easing = callbacks.resolve_easing
                            ? std::optional<std::int16_t>(
                                  callbacks.resolve_easing(easing_index))
                            : std::nullopt;
    if (!easing.has_value()) {
        return WorldEffectRecordAction::easing_unresolved;
    }

    const auto previous_target_x = state.target_x;
    state.target_x = static_cast<std::int16_t>(
        state.target_x + state.velocity_x);

    const auto product = static_cast<std::int32_t>(*easing) *
                         static_cast<std::int32_t>(state.lifetime);
    const auto fixed_bits = static_cast<std::uint32_t>(product) << 6U;
    auto easing_offset = static_cast<std::int16_t>(fixed_bits >> 16U);
    if (state.direction == 0) {
        easing_offset = static_cast<std::int16_t>(-easing_offset);
    }
    state.position_x = static_cast<std::int16_t>(
        easing_offset + state.velocity_x + previous_target_x);
    state.position_y = static_cast<std::int16_t>(
        state.position_y + state.velocity_y);

    state.phase_delay = static_cast<std::uint8_t>(state.phase_delay - 1U);
    if ((state.phase_delay & 0x80U) != 0U) {
        state.phase_delay = 2U;
        state.animation_phase = static_cast<std::uint8_t>(
            state.animation_phase + 1U);
        if (state.animation_phase >= 3U) {
            state.animation_phase = 0U;
        }
    }
    return WorldEffectRecordAction::advanced;
}

std::optional<WorldEffectSpriteSource> resolve_world_effect_sprite(
    const WorldEffectRecordState& state,
    const WorldEffectSpriteCallbacks& callbacks) {
    if (!callbacks.resolve_sprite_source) {
        return std::nullopt;
    }
    const auto token = callbacks.resolve_sprite_source(
        state.source_variant, state.animation_phase);
    if (!token.has_value()) {
        return std::nullopt;
    }
    return WorldEffectSpriteSource{
        token.value(),
        static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(state.position_x) >> 4U),
        static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(state.position_y) >> 4U)};
}

} // namespace shadowrun::ghidra_conversion
