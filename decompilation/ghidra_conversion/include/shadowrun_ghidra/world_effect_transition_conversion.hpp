#pragma once

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct WorldEffectTransitionState {
    std::uint8_t runner_timing = 0; // selected compact record +0x06
    std::uint8_t active_runner_modifier = 0; // FFFBBE +0x00
    std::uint8_t comparison_amount = 0; // FFFBBE +0x01
    std::uint8_t effect_phase = 0; // FFE16A
    std::uint8_t previous_effect_count = 0; // FFF0A4
    std::uint8_t thresholded_rolls = 0; // FFF0DD
    std::uint16_t transition_result = 0; // FFE07E
    bool footer_blocked = false; // FFFBCA != 0
};

struct WorldEffectTransitionCallbacks {
    // ROM 0x55F44 called with D1=-1; returns D6 for the first trial count.
    std::function<std::optional<std::uint16_t>()> calculate_action_delay;
    // ROM 0x0DBA receives D6 as trials and D4 as difficulty.
    std::function<std::optional<std::uint8_t>(
        std::uint16_t trial_count, std::uint16_t difficulty)>
        count_thresholded_random_rolls;
    // ROM 0x19B40: selected HUD mode. It is queried before and after the
    // menu countdown/footer services, matching the source order.
    std::function<std::optional<std::uint8_t>()> select_run_hud_mode;
    // ROM 0x17D90, 0x1A12E, and 0x17DB8 service boundaries.
    std::function<void()> advance_hud_panel;
    std::function<std::optional<std::int8_t>()> advance_menu_countdown;
    std::function<void()> advance_hud_footer_offset;
    // ROM 0x563CC, converted separately as the pending-roll routine.
    std::function<void()> advance_pending_roll;
    // ROM 0x1792E: redraw the field HUD row grid.
    std::function<void()> render_menu_hud_rows;
};

enum class WorldEffectTransitionAction {
    rendered_hud_rows,
    mode_one_selected,
    hud_panel_advanced,
    pending_roll_advanced,
    dependency_unavailable,
};

struct WorldEffectTransitionResult {
    WorldEffectTransitionAction action =
        WorldEffectTransitionAction::dependency_unavailable;
};

// ROM 0x562EC: resolve the mode-2 world-effect transition result and route
// through the source HUD/menu/pending-roll services in their original order.
WorldEffectTransitionResult resolve_world_effect_transition(
    WorldEffectTransitionState& state,
    const WorldEffectTransitionCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
