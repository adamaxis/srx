#include "shadowrun_ghidra/world/world_effect_mode_animation_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

bool decrement_word_until_negative(std::int16_t& value) {
    const auto source_word = static_cast<std::uint16_t>(value);
    const auto next_word = static_cast<std::uint16_t>(source_word - 1U);
    value = static_cast<std::int16_t>(next_word);
    return (next_word & 0x8000U) != 0U;
}

std::uint8_t base_sprite_table_index(
    const WorldEffectModeAnimationState& state) {
    return static_cast<std::uint8_t>((state.render_flags >> 1U) & 0x03U);
}

WorldEffectModeAnimationAction render_sprite(
    WorldEffectModeAnimationState& state,
    const WorldEffectModeAnimationCallbacks& callbacks,
    const std::uint8_t table_index,
    const std::int16_t delta_x,
    const std::int16_t delta_y) {
    if (callbacks.append_sprite_record) {
        callbacks.append_sprite_record(0x06E0U);
    }
    state.position_x = static_cast<std::uint16_t>(
        state.position_x + static_cast<std::uint16_t>(delta_x));
    state.position_y = static_cast<std::uint16_t>(
        state.position_y + static_cast<std::uint16_t>(delta_y));

    const auto token = callbacks.resolve_sprite_data
                           ? callbacks.resolve_sprite_data(table_index)
                           : std::nullopt;
    if (!token.has_value()) {
        return WorldEffectModeAnimationAction::sprite_data_unresolved;
    }
    if (callbacks.build_sprite) {
        callbacks.build_sprite(WorldEffectModeSpriteBuild{
            table_index,
            0x06E0U,
            static_cast<std::uint16_t>(state.position_x >> 4U),
            static_cast<std::uint16_t>(state.position_y >> 4U),
            0x2000U,
            *token});
    }
    return WorldEffectModeAnimationAction::sprite_rendered;
}

WorldEffectModeAnimationAction advance_mode_three(
    WorldEffectModeAnimationState& state,
    const WorldEffectModeAnimationCallbacks& callbacks) {
    if (!decrement_word_until_negative(state.frame_countdown)) {
        if (state.audio_transition_result < 0) {
            if (callbacks.build_title_sprites_twice) {
                callbacks.build_title_sprites_twice();
            }
            return WorldEffectModeAnimationAction::title_sprites_rebuilt;
        }
        return WorldEffectModeAnimationAction::waiting;
    }

    state.mode = 5U;
    state.frame_countdown = 0x10;
    // The source falls directly into mode 5 after publishing these values;
    // do not defer the first mode-5 tick to a later frame.
    if (!decrement_word_until_negative(state.frame_countdown)) {
        if (state.transition_state != 0U) {
            return WorldEffectModeAnimationAction::waiting;
        }
        if (callbacks.initialize_behavior_record) {
            callbacks.initialize_behavior_record();
        }
        if (callbacks.attach_mode_five_behavior_pair) {
            callbacks.attach_mode_five_behavior_pair(
                WorldEffectModeBehaviorPair{0x001C2AEU, 0x001C2A8U});
        }
        return WorldEffectModeAnimationAction::inline_behavior_initialized;
    }

    if (callbacks.attach_behavior_descriptor_e) {
        callbacks.attach_behavior_descriptor_e();
    }
    state.mode = 0U;
    return WorldEffectModeAnimationAction::descriptor_e_attached;
}

WorldEffectModeAnimationAction advance_mode_four_or_five(
    WorldEffectModeAnimationState& state,
    const WorldEffectModeAnimationCallbacks& callbacks) {
    if (decrement_word_until_negative(state.frame_countdown)) {
        if (callbacks.attach_behavior_descriptor_e) {
            callbacks.attach_behavior_descriptor_e();
        }
        state.mode = 0U;
        return WorldEffectModeAnimationAction::descriptor_e_attached;
    }
    if (state.transition_state != 0U) {
        return WorldEffectModeAnimationAction::waiting;
    }

    if (state.mode == 4U) {
        if (callbacks.attach_behavior_descriptor_d) {
            callbacks.attach_behavior_descriptor_d();
        }
        return WorldEffectModeAnimationAction::descriptor_d_attached;
    }

    if (callbacks.initialize_behavior_record) {
        callbacks.initialize_behavior_record();
    }
    if (callbacks.attach_mode_five_behavior_pair) {
        callbacks.attach_mode_five_behavior_pair(
            WorldEffectModeBehaviorPair{0x001C2AEU, 0x001C2A8U});
    }
    return WorldEffectModeAnimationAction::inline_behavior_initialized;
}

WorldEffectModeAnimationAction advance_mode_two(
    WorldEffectModeAnimationState& state,
    const WorldEffectModeAnimationCallbacks& callbacks) {
    if (!decrement_word_until_negative(state.frame_countdown)) {
        auto table_index = static_cast<std::uint8_t>(
            base_sprite_table_index(state) + 4U);
        if (state.frame_countdown < 0x000FU) {
            table_index = static_cast<std::uint8_t>(table_index + 4U);
        }
        return render_sprite(state, callbacks, table_index, -6, 0x1A);
    }

    const auto intro_result = callbacks.advance_intro_state
                                  ? callbacks.advance_intro_state()
                                  : std::nullopt;
    if (!intro_result.has_value()) {
        return WorldEffectModeAnimationAction::intro_state_unresolved;
    }
    state.intro_result = *intro_result;
    if (state.intro_result != 0) {
        state.frame_countdown = 0x10;
        state.mode = 4U;
        return advance_mode_four_or_five(state, callbacks);
    }

    const auto audio_result = callbacks.service_audio_frame
                                  ? callbacks.service_audio_frame(0x18U)
                                  : std::nullopt;
    if (!audio_result.has_value()) {
        return WorldEffectModeAnimationAction::audio_result_unresolved;
    }
    state.audio_transition_result = *audio_result;
    state.frame_countdown = 8;
    state.mode = 3U;
    if (state.title_mode_visible) {
        state.refresh_countdown = 8U;
        state.records_pending = true;
    }
    return advance_mode_three(state, callbacks);
}

} // namespace

WorldEffectModeAnimationAction advance_world_effect_mode_animation(
    WorldEffectModeAnimationState& state,
    const WorldEffectModeAnimationCallbacks& callbacks) {
    switch (state.mode) {
    case 0:
        return WorldEffectModeAnimationAction::inactive;
    case 2:
        return advance_mode_two(state, callbacks);
    case 3:
        return advance_mode_three(state, callbacks);
    case 4:
    case 5:
        return advance_mode_four_or_five(state, callbacks);
    case 6:
        if (decrement_word_until_negative(state.frame_countdown)) {
            if (callbacks.finish_mode_six) {
                callbacks.finish_mode_six();
            }
            state.mode = 0U;
            return WorldEffectModeAnimationAction::mode_finished;
        }
        return render_sprite(state, callbacks, base_sprite_table_index(state),
                             -2, -0x0E);
    default:
        if (!decrement_word_until_negative(state.frame_countdown)) {
            return render_sprite(state, callbacks,
                                 base_sprite_table_index(state), 6, -2);
        }
        state.mode = 2U;
        state.frame_countdown = 0x1E;
        return advance_mode_two(state, callbacks);
    }
}

} // namespace shadowrun::ghidra_conversion
