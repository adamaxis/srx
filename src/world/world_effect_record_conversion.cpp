#include "shadowrun_ghidra/world/world_effect_record_conversion.hpp"

#include <array>

namespace shadowrun::ghidra_conversion {

namespace {

constexpr std::array<std::int16_t, 64> kWorldEffectEasing{
    0,     3211,  6392,  9511,  12539, 15446, 18204, 20787,
    23169, 25329, 27244, 28897, 30272, 31356, 32137, 32609,
    32767, 32609, 32137, 31356, 30272, 28897, 27244, 25329,
    23169, 20787, 18204, 15446, 12539, 9511,  6392,  3211,
    0,     -3211, -6392, -9511, -12539, -15446, -18204, -20787,
    -23169, -25329, -27244, -28897, -30272, -31356, -32137, -32609,
    -32767, -32609, -32137, -31356, -30272, -28897, -27244, -25329,
    -23169, -20787, -18204, -15446, -12539, -9511, -6392, -3211};

constexpr std::array<std::array<std::uint32_t, 3>, 3>
    kWorldEffectSpriteData{{
        {{0x0001D538U, 0x0001D542U, 0x0001D54CU}},
        {{0x0001D556U, 0x0001D560U, 0x0001D56AU}},
        {{0x0001D574U, 0x0001D57EU, 0x0001D588U}},
    }};

} // namespace

std::optional<std::int16_t> resolve_world_effect_easing(
    const std::uint8_t easing_index) {
    if (easing_index >= kWorldEffectEasing.size()) {
        return std::nullopt;
    }
    return kWorldEffectEasing[easing_index];
}

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
        (0x003CU - static_cast<std::uint16_t>(state.lifetime)) & 0x00FFU);
    const auto easing = callbacks.resolve_easing
                            ? std::optional<std::int16_t>(
                                  callbacks.resolve_easing(easing_index))
                            : resolve_world_effect_easing(easing_index);
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
    const auto token = callbacks.resolve_sprite_source
                           ? callbacks.resolve_sprite_source(
                                 state.source_variant,
                                 state.animation_phase)
                           : resolve_world_effect_sprite_data(
                                 state.source_variant,
                                 state.animation_phase);
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

std::optional<std::uint32_t> resolve_world_effect_sprite_data(
    const std::uint8_t source_variant, const std::uint8_t animation_phase) {
    if (animation_phase >= 3U) {
        return std::nullopt;
    }
    const auto variant_index = source_variant == 0U
                                   ? 0U
                                   : source_variant == 1U ? 1U : 2U;
    return kWorldEffectSpriteData[variant_index][animation_phase];
}

WorldEffectSpawnAction spawn_world_effect_record(
    WorldEffectSpawnState& state,
    const WorldEffectSpawnCallbacks& callbacks) {
    if (state.work_record_selected) {
        if (callbacks.clear_work_record) {
            callbacks.clear_work_record();
        }
        state.work_record_selected = false;
    }
    if (state.record.active != 0U) {
        return WorldEffectSpawnAction::record_already_active;
    }

    state.record.direction = callbacks.play_effect_audio
                                 ? callbacks.play_effect_audio()
                                 : 0;
    state.record.active = 0xFFU;
    state.record.phase_delay = 0U;
    state.record.animation_phase = 0U;
    state.record.source_variant = 0U;
    state.record.frame_delay = 0x14U;

    const auto start_x = static_cast<std::int32_t>(state.source_x) +
                         static_cast<std::int32_t>(state.offset_x);
    const auto start_y = static_cast<std::int32_t>(state.source_y) +
                         static_cast<std::int32_t>(state.offset_y);
    const auto start_x_fixed = start_x * 0x10;
    const auto start_y_fixed = start_y * 0x10;
    state.record.target_x = static_cast<std::int16_t>(start_x_fixed);
    state.record.position_y = static_cast<std::int16_t>(start_y_fixed);

    const auto destination_x_fixed =
        static_cast<std::int32_t>(state.destination_x) * 0x10;
    const auto destination_y_fixed =
        static_cast<std::int32_t>(state.destination_y) * 0x10;
    state.record.velocity_x = static_cast<std::int16_t>(
        (destination_x_fixed - start_x_fixed) / 0x3C);
    state.record.velocity_y = static_cast<std::int16_t>(
        (destination_y_fixed - start_y_fixed) / 0x3C);
    state.record.lifetime = 0x3CU;
    return WorldEffectSpawnAction::spawned;
}

} // namespace shadowrun::ghidra_conversion
