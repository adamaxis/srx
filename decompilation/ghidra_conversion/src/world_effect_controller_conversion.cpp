#include "shadowrun_ghidra/world_effect_controller_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

bool decrement_source_byte(std::uint16_t& value) {
    auto byte_value = static_cast<std::uint8_t>(value);
    byte_value = static_cast<std::uint8_t>(byte_value - 1U);
    value = byte_value;
    return (byte_value & 0x80U) != 0U;
}

bool decrement_source_byte(std::uint8_t& value) {
    value = static_cast<std::uint8_t>(value - 1U);
    return (value & 0x80U) != 0U;
}

void reset_effect_entity_locally(WorldEffectControllerState& state) {
    state.descriptor_transition_active = false;
    state.exit_requested = true;
    state.work_record_active = false;
}

std::optional<std::uint16_t> calculate_countdown(
    const WorldEffectControllerCallbacks& callbacks) {
    if (!callbacks.calculate_effect_countdown) {
        return std::nullopt;
    }
    const auto countdown = callbacks.calculate_effect_countdown();
    if (!countdown.has_value() || *countdown == 0U) {
        return std::nullopt;
    }
    return countdown;
}

std::uint16_t source_motion_step(const std::int8_t source_divisor) {
    // The ROM sign-extends FFF0E7 to a word, multiplies the word by 0xF0,
    // and then uses the low word as the DIVU divisor.  Preserve that exact
    // unsigned-word behavior for negative source bytes.
    const auto signed_word = static_cast<std::int16_t>(source_divisor);
    const auto source_word = static_cast<std::uint16_t>(signed_word);
    const auto divisor = static_cast<std::uint16_t>(source_word * 0x00F0U);
    return divisor == 0U ? 0U
                         : static_cast<std::uint16_t>(0x2800U / divisor);
}

} // namespace

WorldEffectTransitionAction advance_world_effect_transition(
    WorldEffectControllerState& state,
    const WorldEffectTransitionCallbacks& callbacks) {
    if (!state.descriptor_transition_active) {
        return WorldEffectTransitionAction::inactive;
    }
    if (state.descriptor_stream_loading) {
        return WorldEffectTransitionAction::descriptor_stream_loading;
    }

    if (decrement_source_byte(state.work.frame_counter)) {
        state.work.frame_counter = 2U;
        if (decrement_source_byte(state.work.descriptor_state)) {
            reset_effect_entity_locally(state);
            if (callbacks.reset_effect_entity) {
                callbacks.reset_effect_entity();
            }
            return WorldEffectTransitionAction::effect_entity_reset;
        }
    }

    if (callbacks.wait_for_vblank) {
        callbacks.wait_for_vblank();
    }
    if (callbacks.stream_transition_noise) {
        callbacks.stream_transition_noise(
            static_cast<std::uint8_t>(state.work.descriptor_state));
    }
    return WorldEffectTransitionAction::noise_frame_streamed;
}

WorldEffectControllerAction advance_world_effect_controller(
    WorldEffectControllerState& state,
    const WorldEffectControllerCallbacks& callbacks) {
    if (!state.work_record_active) {
        return WorldEffectControllerAction::inactive;
    }

    if (state.descriptor_transition_active) {
        const auto transition =
            advance_world_effect_transition(state, callbacks.transition);
        switch (transition) {
        case WorldEffectTransitionAction::descriptor_stream_loading:
            return WorldEffectControllerAction::descriptor_stream_loading;
        case WorldEffectTransitionAction::noise_frame_streamed:
            return WorldEffectControllerAction::transition_noise_streamed;
        case WorldEffectTransitionAction::effect_entity_reset:
            return WorldEffectControllerAction::effect_entity_reset;
        case WorldEffectTransitionAction::inactive:
            break;
        }
    }

    const bool runner_has_effect =
        callbacks.runner_has_active_effect &&
        callbacks.runner_has_active_effect();
    if (!runner_has_effect) {
        if (callbacks.begin_descriptor_transition) {
            callbacks.begin_descriptor_transition();
        }
        return WorldEffectControllerAction::descriptor_transition_started;
    }

    if (!state.world_effects_active) {
        return WorldEffectControllerAction::world_effects_disabled;
    }
    if (state.animation_mode != 0U) {
        return WorldEffectControllerAction::animation_already_active;
    }

    state.effect_countdown = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(state.effect_countdown - 1));
    if (state.effect_countdown >= 0) {
        return WorldEffectControllerAction::countdown_wait;
    }

    const auto countdown = calculate_countdown(callbacks);
    if (!countdown.has_value()) {
        return WorldEffectControllerAction::countdown_unresolved;
    }
    state.effect_countdown = static_cast<std::int16_t>(*countdown);

    if (!callbacks.select_effect_mode) {
        return WorldEffectControllerAction::effect_mode_unresolved;
    }
    const auto selected_mode = callbacks.select_effect_mode();
    if (!selected_mode.has_value()) {
        return WorldEffectControllerAction::effect_mode_unresolved;
    }
    state.selected_effect_mode = *selected_mode;
    if (*selected_mode < 0) {
        return WorldEffectControllerAction::effect_mode_rejected;
    }

    if (*selected_mode == 2) {
        const auto background_divisor = calculate_countdown(callbacks);
        if (!background_divisor.has_value()) {
            return WorldEffectControllerAction::countdown_unresolved;
        }
        state.background_step = static_cast<std::uint16_t>(
            0x0A00U / *background_divisor);
        state.background_started = 1U;
        state.background_position = 0x11F0U;
        return WorldEffectControllerAction::background_effect_initialized;
    }

    if (*selected_mode == 3) {
        state.motion_step = source_motion_step(state.animation_divisor == 0
                                                   ? static_cast<std::int8_t>(1)
                                                   : state.animation_divisor);
        if (state.world_effect_mode == 6) {
            state.secondary_started = 2U;
        } else if (state.world_effect_mode == 7) {
            state.primary_started = 2U;
        }
        return WorldEffectControllerAction::motion_effect_initialized;
    }

    if (callbacks.initialize_effect_defaults) {
        callbacks.initialize_effect_defaults();
    }
    state.animation_mode = 2U;
    state.title_phase_handler_index = 2U;
    return WorldEffectControllerAction::default_effect_initialized;
}

} // namespace shadowrun::ghidra_conversion
