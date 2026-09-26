#include "shadowrun_ghidra/world/world_effect_animation_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

bool decrement_source_byte(std::uint16_t& value) {
    auto byte_value = static_cast<std::uint8_t>(value);
    byte_value = static_cast<std::uint8_t>(byte_value - 1U);
    value = byte_value;
    return (byte_value & 0x80U) != 0U;
}

std::uint8_t add_runner_delta_as_source_byte(
    const std::uint8_t frame_value,
    const std::int16_t delta) {
    // 0x1AAAC loads the frame as a byte and performs ADD.B D1b,D0b. The
    // following BPL tests the wrapped byte, not the widened signed table
    // value. Keep that distinction explicit for malformed/custom table
    // boundaries as well as the normal ROM delta table.
    const auto wrapped = static_cast<std::uint8_t>(
        static_cast<std::uint16_t>(frame_value) +
        static_cast<std::uint8_t>(delta));
    return (wrapped & 0x80U) == 0U ? wrapped : 0U;
}

WorldEffectAnimationAction render_animation_sprite(
    WorldEffectAnimationState& state,
    const WorldEffectAnimationCallbacks& callbacks,
    const bool transition_table,
    const std::uint32_t source_table_token) {
    const auto phase = static_cast<std::uint8_t>(
        state.work.phase_or_descriptor_index);
    const auto token = callbacks.resolve_sprite_data
                           ? callbacks.resolve_sprite_data(
                                 transition_table, source_table_token, phase)
                           : std::nullopt;
    if (!token.has_value()) {
        return WorldEffectAnimationAction::sprite_data_unresolved;
    }

    if (callbacks.append_sprite_record) {
        callbacks.append_sprite_record(0x0694U);
    }
    WorldEffectAnimationSpriteBuild build{};
    build.transition_table = transition_table;
    build.source_table_token = source_table_token;
    build.phase = phase;
    build.x = state.work.x;
    build.y = state.work.y;
    build.sprite_data_token = *token;
    if (callbacks.build_sprite) {
        callbacks.build_sprite(build);
    }
    return WorldEffectAnimationAction::rendered;
}

} // namespace

WorldEffectAnimationAction tick_world_effect_animation(
    WorldEffectAnimationState& state,
    const WorldEffectAnimationCallbacks& callbacks) {
    if (!state.animation_active) {
        return WorldEffectAnimationAction::inactive;
    }

    if (!state.descriptor_loading && !state.transition_active) {
        const bool use_runner_timing =
            !state.ffdf8c_nonzero || state.selected_mode != 0;
        if (use_runner_timing) {
            if (decrement_source_byte(state.work.frame_counter)) {
                const auto delta = callbacks.read_runner_animation_delta
                                       ? callbacks.read_runner_animation_delta()
                                       : std::nullopt;
                if (!delta.has_value()) {
                    return WorldEffectAnimationAction::animation_delta_unresolved;
                }
                state.work.frame_counter = add_runner_delta_as_source_byte(
                    state.work.descriptor_state, *delta);
                state.work.phase_or_descriptor_index = static_cast<std::uint8_t>(
                    state.work.phase_or_descriptor_index + 1U);
                if (state.work.phase_or_descriptor_index >=
                    state.work.descriptor_phase) {
                    state.work.phase_or_descriptor_index = 0U;
                }
            }
        } else if (decrement_source_byte(state.work.frame_counter)) {
            state.work.frame_counter = state.work.descriptor_state;
            state.work.phase_or_descriptor_index = static_cast<std::uint8_t>(
                state.work.phase_or_descriptor_index + 1U);
            if (state.work.phase_or_descriptor_index > 3U) {
                state.work.phase_or_descriptor_index = 3U;
            }
        }

        const auto rendered = render_animation_sprite(
            state, callbacks, false, state.work.descriptor_pointer);
        if (rendered != WorldEffectAnimationAction::rendered) {
            return rendered;
        }
        if (callbacks.advance_primary_overlay) {
            callbacks.advance_primary_overlay();
        }
        if (callbacks.advance_secondary_overlay) {
            callbacks.advance_secondary_overlay();
        }
        if (callbacks.advance_background_overlay) {
            callbacks.advance_background_overlay();
        }
        return WorldEffectAnimationAction::rendered;
    }

    if (decrement_source_byte(state.work.frame_counter)) {
        state.work.frame_counter = 5U;
        state.work.phase_or_descriptor_index = static_cast<std::uint8_t>(
            state.work.phase_or_descriptor_index + 1U);
        if (state.work.phase_or_descriptor_index >= 6U) {
            if (callbacks.reset_effect_entity) {
                callbacks.reset_effect_entity();
            }
            return WorldEffectAnimationAction::effect_entity_reset;
        }
    }

    constexpr std::uint32_t kTransitionSpriteTable = 0x000F1C46U;
    return render_animation_sprite(state, callbacks, true,
                                   kTransitionSpriteTable);
}

} // namespace shadowrun::ghidra_conversion
