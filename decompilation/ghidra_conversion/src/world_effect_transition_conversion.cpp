#include "shadowrun_ghidra/world_effect_transition_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

void render_rows(const WorldEffectTransitionCallbacks& callbacks) {
    if (callbacks.render_menu_hud_rows) {
        callbacks.render_menu_hud_rows();
    }
}

bool is_footer_sensitive_mode(const std::uint8_t mode) {
    return mode == 3U || mode == 12U || mode == 15U || mode == 23U ||
           mode == 24U;
}

} // namespace

WorldEffectTransitionResult resolve_world_effect_transition(
    WorldEffectTransitionState& state,
    const WorldEffectTransitionCallbacks& callbacks) {
    if (!callbacks.calculate_action_delay ||
        !callbacks.count_thresholded_random_rolls ||
        !callbacks.select_run_hud_mode) {
        return {};
    }
    const auto action_delay = callbacks.calculate_action_delay();
    if (!action_delay.has_value()) {
        return {};
    }

    const auto trial_count = static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(*action_delay) & 0xFF00U) |
        static_cast<std::uint8_t>(static_cast<std::uint8_t>(*action_delay) +
                                  state.active_runner_modifier));
    auto difficulty = static_cast<std::uint16_t>(
        state.runner_timing + static_cast<std::uint16_t>(state.effect_phase) *
                                 2U);
    if (static_cast<std::int16_t>(difficulty - 2U) < 0) {
        difficulty = 2U;
    }
    const auto rolls = callbacks.count_thresholded_random_rolls(
        trial_count, difficulty);
    if (!rolls.has_value()) {
        return {};
    }
    state.thresholded_rolls = *rolls;

    const auto signed_difference = static_cast<std::int8_t>(
        static_cast<std::uint8_t>(state.previous_effect_count -
                                  state.thresholded_rolls));
    if (signed_difference == 0) {
        render_rows(callbacks);
        return {WorldEffectTransitionAction::rendered_hud_rows};
    }
    if (signed_difference < 0) {
        state.transition_result = 1U;
        render_rows(callbacks);
        return {WorldEffectTransitionAction::mode_one_selected};
    }

    const auto remaining_check = static_cast<std::int8_t>(
        static_cast<std::uint8_t>(state.comparison_amount -
                                  static_cast<std::uint8_t>(signed_difference)));
    if (remaining_check >= 0) {
        render_rows(callbacks);
        return {WorldEffectTransitionAction::rendered_hud_rows};
    }

    const auto selected_mode = callbacks.select_run_hud_mode();
    if (!selected_mode.has_value()) {
        return {};
    }
    if (*selected_mode == 8U) {
        if (callbacks.advance_hud_panel) {
            callbacks.advance_hud_panel();
        }
        return {WorldEffectTransitionAction::hud_panel_advanced};
    }

    if (!callbacks.advance_menu_countdown) {
        return {};
    }
    const auto countdown_result = callbacks.advance_menu_countdown();
    if (!countdown_result.has_value()) {
        return {};
    }
    if (*countdown_result >= 0) {
        render_rows(callbacks);
        return {WorldEffectTransitionAction::rendered_hud_rows};
    }

    if (callbacks.advance_hud_footer_offset) {
        callbacks.advance_hud_footer_offset();
    }
    if (state.footer_blocked) {
        render_rows(callbacks);
        return {WorldEffectTransitionAction::rendered_hud_rows};
    }

    const auto footer_mode = callbacks.select_run_hud_mode();
    if (!footer_mode.has_value()) {
        return {};
    }
    if (is_footer_sensitive_mode(*footer_mode)) {
        if (callbacks.advance_pending_roll) {
            callbacks.advance_pending_roll();
            return {WorldEffectTransitionAction::pending_roll_advanced};
        }
        return {};
    }

    render_rows(callbacks);
    return {WorldEffectTransitionAction::rendered_hud_rows};
}

} // namespace shadowrun::ghidra_conversion
