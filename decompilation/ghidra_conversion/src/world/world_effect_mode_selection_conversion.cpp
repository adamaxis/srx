#include "shadowrun_ghidra/world/world_effect_mode_selection_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {
namespace {

std::optional<std::uint8_t> run_thresholded_roll(
    WorldEffectModeSelectionState& state,
    const WorldEffectModeSelectionCallbacks& callbacks,
    const std::uint8_t base_value,
    const std::uint16_t trial_count,
    const bool phase_only) {
    // ROM 0x562D4 has no return before 0x562DE: callers entering at the
    // timer entry receive both the timer and phase modifiers. The transition
    // routine enters 0x562DE directly and receives phase only.
    const auto modified = phase_only
                              ? add_world_effect_phase_modifier(
                                    base_value, state.effect_phase)
                              : apply_world_effect_timing_modifiers(
                                    base_value, state.effect_timer,
                                    state.effect_phase);
    const auto difficulty = std::max<std::uint16_t>(2U, modified);
    if (!callbacks.count_thresholded_random_rolls) {
        return std::nullopt;
    }
    const auto result = callbacks.count_thresholded_random_rolls(
        trial_count, difficulty);
    if (!result.has_value()) {
        return std::nullopt;
    }
    state.thresholded_rolls = static_cast<std::int8_t>(*result);
    return result;
}

std::uint16_t add_active_modifier(const std::uint16_t value,
                                  const std::uint8_t modifier) {
    auto result = value;
    const auto low = static_cast<std::uint8_t>(result);
    result = static_cast<std::uint16_t>(
        (result & 0xFF00U) | static_cast<std::uint8_t>(low + modifier));
    return result;
}

bool source_subtract_byte_is_nonpositive(const std::uint8_t minuend,
                                         const std::uint8_t subtrahend,
                                         const std::uint8_t result) {
    // 0x561A0 is SUB.B followed by BLE. Preserve the source N/V/Z decision;
    // a signed C++ cast is wrong when the byte subtraction overflows.
    const bool negative = (result & 0x80U) != 0U;
    const bool zero = result == 0U;
    const bool overflow =
        (((minuend ^ subtrahend) & 0x80U) != 0U) &&
        (((minuend ^ result) & 0x80U) != 0U);
    return zero || (negative != overflow);
}

WorldEffectIntroGateResult finish_intro_marker(
    WorldEffectModeSelectionState& state,
    const WorldEffectModeSelectionCallbacks& callbacks) {
    if (state.mode_variant > 0) {
        if (callbacks.initialize_world_effect_mode) {
            callbacks.initialize_world_effect_mode();
        }
        state.intro_marker = 0;
        return { -1,
                 WorldEffectIntroGateAction::world_effect_mode_initialized };
    }
    state.world_effects_active = false;
    state.intro_marker = 0;
    return { -1, WorldEffectIntroGateAction::world_effects_disabled };
}

WorldEffectIntroGateResult run_intro_transition_rolls(
    WorldEffectModeSelectionState& state,
    const WorldEffectModeSelectionCallbacks& callbacks) {
    const auto first = run_thresholded_roll(
        state, callbacks, callbacks.inputs.intro_threshold,
        state.initial_trial_count, true);
    if (!first.has_value()) {
        return {-1, WorldEffectIntroGateAction::random_result_unresolved};
    }
    const auto first_count = *first;

    if (!callbacks.calculate_action_delay) {
        return {-1, WorldEffectIntroGateAction::action_delay_unresolved};
    }
    const auto delay = callbacks.calculate_action_delay();
    if (!delay.has_value()) {
        return {-1, WorldEffectIntroGateAction::action_delay_unresolved};
    }
    const auto second_trial_count =
        add_active_modifier(*delay, state.active_record_modifier);
    const auto second = run_thresholded_roll(
        state, callbacks, state.current_record.intro_timing_value,
        second_trial_count, true);
    if (!second.has_value()) {
        return {-1, WorldEffectIntroGateAction::random_result_unresolved};
    }

    const auto difference = static_cast<std::uint8_t>(first_count - *second);
    if (!source_subtract_byte_is_nonpositive(
            first_count, *second, difference)) {
        if (callbacks.update_hud_mode_transition) {
            callbacks.update_hud_mode_transition(1);
        }
        // The source checks FFF0B1 after 0x1989A, then branches directly to
        // the marker-clear exit when that service has blocked the transition.
        if (state.hud_transition_blocked) {
            state.intro_marker = 0;
            return {-1, WorldEffectIntroGateAction::hud_transition_blocked};
        }
    }

    if (callbacks.run_menu_entry_service) {
        callbacks.run_menu_entry_service();
    }
    return finish_intro_marker(state, callbacks);
}

WorldEffectModeSelectionResult run_action_count(
    WorldEffectModeSelectionState& state,
    const WorldEffectModeSelectionCallbacks& callbacks) {
    const auto result = run_thresholded_roll(
        state, callbacks, callbacks.inputs.action_threshold,
        state.initial_trial_count, false);
    if (!result.has_value()) {
        return {-1, WorldEffectModeSelectionAction::random_result_unresolved};
    }

    if (callbacks.select_run_hud_mode) {
        const auto mode = callbacks.select_run_hud_mode();
        if (mode.has_value() && *mode == 8U) {
            if (callbacks.draw_hud_panel) {
                callbacks.draw_hud_panel();
            }
            if (callbacks.advance_hud_panel) {
                callbacks.advance_hud_panel();
            }
        }
    }

    state.last_effect_count = state.thresholded_rolls;
    if (state.thresholded_rolls <= 0) {
        state.last_effect_count = 1;
        return {-1, WorldEffectModeSelectionAction::action_count};
    }
    return {0, WorldEffectModeSelectionAction::action_count};
}

WorldEffectModeSelectionResult run_delayed_effect(
    WorldEffectModeSelectionState& state,
    const WorldEffectModeSelectionCallbacks& callbacks) {
    if (state.delayed_effect_gate < 0) {
        state.motion_divisor = static_cast<std::int8_t>(
            static_cast<std::uint8_t>(state.motion_divisor - 1));
        return {-1, WorldEffectModeSelectionAction::delayed_effect};
    }

    // 0x560CE enters 0x562D4, so delayed effects include both FFE169 and
    // twice FFE16A. Intro rolls enter 0x562DE directly and remain phase-only.
    const auto result = run_thresholded_roll(
        state, callbacks, state.current_record.delayed_timing_value,
        state.initial_trial_count, false);
    if (!result.has_value()) {
        return {-1, WorldEffectModeSelectionAction::random_result_unresolved};
    }
    state.thresholded_rolls = static_cast<std::int8_t>(
        static_cast<std::uint8_t>(state.thresholded_rolls - 1));
    if (state.thresholded_rolls < 0) {
        return {-1, WorldEffectModeSelectionAction::delayed_effect};
    }

    state.motion_divisor = static_cast<std::int8_t>(
        static_cast<std::uint8_t>(10U -
                                  static_cast<std::uint8_t>(
                                      state.thresholded_rolls)));
    state.delayed_effect_gate = -1;
    return {3, WorldEffectModeSelectionAction::delayed_effect};
}

} // namespace

std::uint16_t apply_world_effect_timing_modifiers(
    const std::uint16_t base_value,
    const std::uint8_t effect_timer,
    const std::uint8_t effect_phase) {
    return add_world_effect_phase_modifier(
        add_world_effect_timer_modifier(base_value, effect_timer),
        effect_phase);
}

std::uint16_t add_world_effect_timer_modifier(
    const std::uint16_t base_value,
    const std::uint8_t effect_timer) {
    return static_cast<std::uint16_t>(
        base_value + static_cast<std::uint16_t>(effect_timer));
}

std::uint16_t add_world_effect_phase_modifier(
    const std::uint16_t base_value,
    const std::uint8_t effect_phase) {
    return static_cast<std::uint16_t>(
        base_value + static_cast<std::uint16_t>(effect_phase) * 2U);
}

WorldEffectIntroGateResult advance_world_effect_intro_gate(
    WorldEffectModeSelectionState& state,
    const WorldEffectModeSelectionCallbacks& callbacks) {
    if (state.intro_marker < 0) {
        return run_intro_transition_rolls(state, callbacks);
    }
    if (state.intro_marker > 0) {
        state.intro_marker = 0;
        return {-1, WorldEffectIntroGateAction::marker_cleared};
    }
    if (state.mode_variant == 1) {
        return {-1, WorldEffectIntroGateAction::already_in_intro_variant};
    }
    state.intro_marker = -1;
    return {2, WorldEffectIntroGateAction::mode_two_started};
}

WorldEffectModeSelectionResult select_world_effect_animation_mode(
    WorldEffectModeSelectionState& state,
    const WorldEffectModeSelectionCallbacks& callbacks) {
    if (!callbacks.read_secondary_animation_index) {
        return {-1, WorldEffectModeSelectionAction::secondary_record_unresolved};
    }
    const auto secondary_index = callbacks.read_secondary_animation_index();
    if (!secondary_index.has_value()) {
        return {-1, WorldEffectModeSelectionAction::secondary_record_unresolved};
    }
    if (*secondary_index != state.current_record.animation_index) {
        state.current_record.animation_index =
            static_cast<std::uint8_t>(state.current_record.animation_index + 1U);
    }

    const auto status = state.current_record.effect_status;
    if (status < 2) {
        const auto intro = advance_world_effect_intro_gate(state, callbacks);
        return {intro.selected_mode,
                intro.action == WorldEffectIntroGateAction::random_result_unresolved
                    ? WorldEffectModeSelectionAction::random_result_unresolved
                : intro.action == WorldEffectIntroGateAction::action_delay_unresolved
                    ? WorldEffectModeSelectionAction::action_delay_unresolved
                    : WorldEffectModeSelectionAction::intro_gate};
    }
    if (status >= 2 && status < 5) {
        return run_action_count(state, callbacks);
    }
    if (status >= 5 && status < 7) {
        return run_delayed_effect(state, callbacks);
    }
    if (status >= 7 && status < 11) {
        if (state.mode_variant > 0) {
            return run_delayed_effect(state, callbacks);
        }
        const auto intro = advance_world_effect_intro_gate(state, callbacks);
        return {intro.selected_mode,
                intro.action == WorldEffectIntroGateAction::random_result_unresolved
                    ? WorldEffectModeSelectionAction::random_result_unresolved
                : intro.action == WorldEffectIntroGateAction::action_delay_unresolved
                    ? WorldEffectModeSelectionAction::action_delay_unresolved
                    : WorldEffectModeSelectionAction::intro_gate};
    }
    if (status >= 11 && status < 17) {
        if (state.mode_variant < 1) {
            const auto intro = advance_world_effect_intro_gate(state, callbacks);
            return {intro.selected_mode,
                    intro.action == WorldEffectIntroGateAction::random_result_unresolved
                        ? WorldEffectModeSelectionAction::random_result_unresolved
                    : intro.action == WorldEffectIntroGateAction::action_delay_unresolved
                        ? WorldEffectModeSelectionAction::action_delay_unresolved
                        : WorldEffectModeSelectionAction::intro_gate};
        }
        return run_action_count(state, callbacks);
    }
    if (status >= 17 && status < 21) {
        const auto intro = advance_world_effect_intro_gate(state, callbacks);
        return {intro.selected_mode,
                intro.action == WorldEffectIntroGateAction::random_result_unresolved
                    ? WorldEffectModeSelectionAction::random_result_unresolved
                : intro.action == WorldEffectIntroGateAction::action_delay_unresolved
                    ? WorldEffectModeSelectionAction::action_delay_unresolved
                    : WorldEffectModeSelectionAction::intro_gate};
    }
    return run_action_count(state, callbacks);
}

} // namespace shadowrun::ghidra_conversion
