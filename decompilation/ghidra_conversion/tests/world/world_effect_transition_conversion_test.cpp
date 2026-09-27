#include "shadowrun_ghidra/world/world_effect_transition_conversion.hpp"

#include <cstdint>
#include <optional>

int main() {
    using namespace shadowrun::ghidra_conversion;

    WorldEffectTransitionState state{};
    state.runner_timing = 4U;
    state.active_runner_modifier = 1U;
    state.effect_phase = 9U;
    state.previous_effect_count = 5U;
    int rolls = 0;
    int renders = 0;
    WorldEffectTransitionCallbacks callbacks{};
    callbacks.calculate_action_delay = [] {
        return std::optional<std::uint16_t>(6U);
    };
    callbacks.count_thresholded_random_rolls =
        [&](const std::uint16_t trials,
            const std::uint16_t difficulty) -> std::optional<std::uint8_t> {
        ++rolls;
        return trials == 7U && difficulty == 22U
                   ? std::optional<std::uint8_t>(5U)
                   : std::nullopt;
    };
    callbacks.select_run_hud_mode = [] {
        return std::optional<std::uint8_t>(7U);
    };
    callbacks.render_menu_hud_rows = [&] { ++renders; };
    callbacks.advance_menu_countdown = [] {
        return std::optional<std::int8_t>(0);
    };

    const auto equal = resolve_world_effect_transition(state, callbacks);
    if (equal.action != WorldEffectTransitionAction::rendered_hud_rows ||
        renders != 1 || rolls != 1 || state.thresholded_rolls != 5U) {
        return 1;
    }

    state.previous_effect_count = 4U;
    callbacks.count_thresholded_random_rolls =
        [](const std::uint16_t, const std::uint16_t) {
            return std::optional<std::uint8_t>(5U);
        };
    const auto mode_one = resolve_world_effect_transition(state, callbacks);
    if (mode_one.action != WorldEffectTransitionAction::mode_one_selected ||
        state.transition_result != 1U || renders != 2) {
        return 2;
    }

    state.previous_effect_count = 8U;
    state.comparison_amount = 2U;
    int hud_advances = 0;
    callbacks.select_run_hud_mode = [] {
        return std::optional<std::uint8_t>(8U);
    };
    callbacks.advance_hud_panel = [&] { ++hud_advances; };
    const auto hud_panel = resolve_world_effect_transition(state, callbacks);
    if (hud_panel.action != WorldEffectTransitionAction::hud_panel_advanced ||
        hud_advances != 1 || renders != 2) {
        return 3;
    }

    callbacks.select_run_hud_mode = [] {
        return std::optional<std::uint8_t>(7U);
    };
    callbacks.advance_menu_countdown = [] {
        return std::optional<std::int8_t>(1);
    };
    const auto countdown = resolve_world_effect_transition(state, callbacks);
    if (countdown.action != WorldEffectTransitionAction::rendered_hud_rows ||
        renders != 3) {
        return 4;
    }

    int footer_advances = 0;
    int pending_rolls = 0;
    callbacks.advance_menu_countdown = [] {
        return std::optional<std::int8_t>(-1);
    };
    callbacks.advance_hud_footer_offset = [&] { ++footer_advances; };
    callbacks.advance_pending_roll = [&] { ++pending_rolls; };
    callbacks.select_run_hud_mode = [] {
        return std::optional<std::uint8_t>(3U);
    };
    const auto pending = resolve_world_effect_transition(state, callbacks);
    if (pending.action != WorldEffectTransitionAction::pending_roll_advanced ||
        footer_advances != 1 || pending_rolls != 1 || renders != 3) {
        return 5;
    }

    state.footer_blocked = true;
    const auto blocked = resolve_world_effect_transition(state, callbacks);
    if (blocked.action != WorldEffectTransitionAction::rendered_hud_rows ||
        renders != 4) {
        return 6;
    }

    // The early equality path reaches the row redraw without selecting a HUD
    // mode; that service is only required after the comparison gate.
    WorldEffectTransitionState early{};
    early.previous_effect_count = 5U;
    auto early_callbacks = callbacks;
    early_callbacks.select_run_hud_mode = {};
    early_callbacks.advance_menu_countdown = {};
    const auto early_result = resolve_world_effect_transition(
        early, early_callbacks);
    if (early_result.action != WorldEffectTransitionAction::rendered_hud_rows ||
        renders != 5) {
        return 7;
    }

    // 0x56330 uses overflow-aware BGT. 0x80 - 0x01 has a wrapped 0x7f
    // result, but V is set, so the source takes the negative/mode-one path.
    WorldEffectTransitionState overflow{};
    overflow.previous_effect_count = 0x80U;
    overflow.comparison_amount = 0U;
    auto overflow_callbacks = early_callbacks;
    overflow_callbacks.count_thresholded_random_rolls =
        [](const std::uint16_t, const std::uint16_t) {
            return std::optional<std::uint8_t>(1U);
        };
    const auto overflow_result = resolve_world_effect_transition(
        overflow, overflow_callbacks);
    if (overflow_result.action != WorldEffectTransitionAction::mode_one_selected ||
        overflow.transition_result != 1U || renders != 6) {
        return 8;
    }

    WorldEffectTransitionCallbacks unavailable{};
    if (resolve_world_effect_transition(state, unavailable).action !=
        WorldEffectTransitionAction::dependency_unavailable) {
        return 9;
    }
    return 0;
}
