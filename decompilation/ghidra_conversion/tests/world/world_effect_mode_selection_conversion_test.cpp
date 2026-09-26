#include "shadowrun_ghidra/world/world_effect_mode_selection_conversion.hpp"

#include <cstdint>
#include <optional>

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (apply_world_effect_timing_modifiers(4U, 3U, 2U) != 11U) {
        return 1;
    }
    if (add_world_effect_timer_modifier(4U, 3U) != 7U ||
        add_world_effect_phase_modifier(4U, 2U) != 8U) {
        return 8;
    }

    WorldEffectModeSelectionState start{};
    start.mode_variant = 0;
    start.intro_marker = 0;
    WorldEffectModeSelectionCallbacks no_rolls{};
    no_rolls.read_secondary_animation_index = [] {
        return std::optional<std::uint8_t>(4U);
    };
    const auto started = select_world_effect_animation_mode(start, no_rolls);
    if (started.selected_mode != 2 ||
        started.action != WorldEffectModeSelectionAction::intro_gate ||
        start.intro_marker != -1) {
        return 2;
    }

    WorldEffectModeSelectionState variant_one{};
    variant_one.mode_variant = 1;
    variant_one.intro_marker = 0;
    WorldEffectModeSelectionCallbacks variant_callbacks = no_rolls;
    if (advance_world_effect_intro_gate(variant_one, variant_callbacks)
            .selected_mode != -1 ||
        variant_one.intro_marker != 0) {
        return 3;
    }

    WorldEffectModeSelectionState delayed{};
    delayed.current_record.effect_status = 5;
    delayed.current_record.delayed_timing_value = 4U;
    delayed.effect_timer = 1U;
    delayed.effect_phase = 9U;
    delayed.active_record_modifier = 2U;
    delayed.thresholded_rolls = 0;
    int roll_calls = 0;
    WorldEffectModeSelectionCallbacks delayed_callbacks{};
    delayed_callbacks.read_secondary_animation_index = [] {
        return std::optional<std::uint8_t>(0U);
    };
    delayed_callbacks.count_thresholded_random_rolls =
        [&](const std::uint16_t trials,
            const std::uint16_t difficulty) -> std::optional<std::uint8_t> {
        ++roll_calls;
            return trials == 0U && difficulty == 5U
                   ? std::optional<std::uint8_t>(2U)
                   : std::nullopt;
    };
    const auto delayed_result =
        select_world_effect_animation_mode(delayed, delayed_callbacks);
    if (delayed_result.selected_mode != 3 || roll_calls != 1 ||
        delayed.motion_divisor != 9 || delayed.delayed_effect_gate != -1 ||
        delayed.thresholded_rolls != 1) {
        return 40 + (delayed_result.selected_mode == 3 ? 0 : 1) +
               (roll_calls == 1 ? 0 : 2) +
               (delayed.motion_divisor == 9 ? 0 : 4) +
               (delayed.delayed_effect_gate == -1 ? 0 : 8) +
               (delayed.thresholded_rolls == 1 ? 0 : 16);
    }

    WorldEffectModeSelectionState action{};
    action.current_record.effect_status = 3;
    action.mode_variant = 1;
    int draws = 0;
    int advances = 0;
    WorldEffectModeSelectionCallbacks action_callbacks{};
    action_callbacks.read_secondary_animation_index = [] {
        return std::optional<std::uint8_t>(0U);
    };
    action_callbacks.count_thresholded_random_rolls =
        [](const std::uint16_t, const std::uint16_t) {
            return std::optional<std::uint8_t>(1U);
        };
    action_callbacks.select_run_hud_mode = [] {
        return std::optional<std::uint8_t>(8U);
    };
    action_callbacks.draw_hud_panel = [&] { ++draws; };
    action_callbacks.advance_hud_panel = [&] { ++advances; };
    const auto action_result =
        select_world_effect_animation_mode(action, action_callbacks);
    if (action_result.selected_mode != 0 || draws != 1 || advances != 1 ||
        action.last_effect_count != 1) {
        return 5;
    }

    WorldEffectModeSelectionState intro{};
    intro.intro_marker = -1;
    intro.mode_variant = 1;
    intro.current_record.intro_timing_value = 5U;
    intro.active_record_modifier = 1U;
    intro.effect_timer = 9U;
    int hud_updates = 0;
    int menu_services = 0;
    int mode_initializations = 0;
    WorldEffectModeSelectionCallbacks intro_callbacks{};
    intro_callbacks.inputs.intro_threshold = 4U;
    intro_callbacks.read_secondary_animation_index = [] {
        return std::optional<std::uint8_t>(0U);
    };
    intro_callbacks.count_thresholded_random_rolls =
        [&](const std::uint16_t trials,
            const std::uint16_t difficulty) -> std::optional<std::uint8_t> {
        if (trials == 0U && difficulty == 4U) {
            return 3U;
        }
        if (trials == 7U && difficulty == 5U) {
            return 1U;
        }
        return std::nullopt;
    };
    intro_callbacks.calculate_action_delay =
        [] { return std::optional<std::uint16_t>(6U); };
    intro_callbacks.update_hud_mode_transition = [&](const std::int16_t delta) {
        hud_updates += delta == 1 ? 1 : 100;
    };
    intro_callbacks.run_menu_entry_service = [&] { ++menu_services; };
    intro_callbacks.initialize_world_effect_mode = [&] { ++mode_initializations; };
    const auto intro_result =
        advance_world_effect_intro_gate(intro, intro_callbacks);
    if (intro_result.selected_mode != -1 || hud_updates != 1 ||
        menu_services != 1 || mode_initializations != 1 ||
        intro.intro_marker != 0) {
        return 6;
    }

    WorldEffectModeSelectionState blocked{};
    blocked.intro_marker = -1;
    blocked.hud_transition_blocked = true;
    blocked.current_record.intro_timing_value = 2U;
    WorldEffectModeSelectionCallbacks blocked_callbacks = intro_callbacks;
    blocked_callbacks.count_thresholded_random_rolls =
        [](const std::uint16_t, const std::uint16_t difficulty) {
            return std::optional<std::uint8_t>(difficulty == 4U ? 3U : 2U);
        };
    blocked_callbacks.calculate_action_delay =
        [] { return std::optional<std::uint16_t>(0U); };
    const auto blocked_result =
        advance_world_effect_intro_gate(blocked, blocked_callbacks);
    if (blocked_result.action !=
            WorldEffectIntroGateAction::hud_transition_blocked ||
        blocked.intro_marker != -1) {
        return 7;
    }

    return 0;
}
