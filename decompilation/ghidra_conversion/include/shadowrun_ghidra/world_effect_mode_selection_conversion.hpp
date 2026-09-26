#pragma once

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

// The compact FF1100 roster record fields consumed by 0x56116/0x561E8.
// They are deliberately exposed as record roles, not source-address slots.
struct WorldEffectSelectionRecord {
    std::uint8_t animation_index = 0; // +0x10
    std::int8_t effect_status = 0; // +0x13
    std::uint8_t intro_timing_value = 0; // +0x06
    std::uint8_t delayed_timing_value = 0; // +0x09
};

struct WorldEffectModeSelectionState {
    WorldEffectSelectionRecord current_record{};
    std::int8_t intro_marker = 0; // FFF0A3
    std::int8_t mode_variant = 0; // FFF0A0
    std::int8_t thresholded_rolls = 0; // FFF0DD
    std::int8_t delayed_effect_gate = 0; // FFF0E6
    std::int8_t motion_divisor = 0; // FFF0E7
    std::int8_t last_effect_count = 0; // FFF0A4
    bool world_effects_active = false; // FFDF84 != 0
    bool hud_transition_blocked = false; // FFF0B1 < 0
    std::uint8_t effect_timer = 0; // FFE169
    std::uint8_t effect_phase = 0; // FFE16A
    std::uint8_t active_record_modifier = 0; // FFFC04 + 0x88
    std::uint16_t initial_trial_count = 0; // D6 at the selector entry
};

struct WorldEffectModeSelectionInputs {
    std::uint8_t action_threshold = 0; // FFFBBE + 0x08
    std::uint8_t delayed_threshold = 0; // FFFBBE + 0x09
    std::uint8_t intro_threshold = 0; // FFFBBE + 0x0A
};

struct WorldEffectModeSelectionCallbacks {
    // ROM 0x53216 returns the selected secondary compact record's +0x10.
    std::function<std::optional<std::uint8_t>()>
        read_secondary_animation_index;
    WorldEffectModeSelectionInputs inputs{};

    // ROM 0x0DBA receives D6 as the trial count and D4 as the difficulty,
    // then publishes FFF0DD. The host supplies the source RNG at this seam.
    std::function<std::optional<std::uint8_t>(
        std::uint16_t trial_count, std::uint16_t difficulty)>
        count_thresholded_random_rolls;
    // ROM 0x55F44 called with D1=-1; result is its D6 delay before the caller
    // adds the active runner's +0x88 byte for the second trial count.
    std::function<std::optional<std::uint16_t>()> calculate_action_delay;

    // ROM 0x19B40, 0x1989A, 0x17A5E, and 0x1A908 remain typed service
    // boundaries. Their order is part of the source behavior.
    std::function<std::optional<std::uint8_t>()> select_run_hud_mode;
    std::function<void(std::int16_t delta)> update_hud_mode_transition;
    std::function<void()> run_menu_entry_service;
    std::function<void()> initialize_world_effect_mode;
    std::function<void()> draw_hud_panel;
    std::function<void()> advance_hud_panel;
};

enum class WorldEffectIntroGateAction {
    mode_two_started,
    already_in_intro_variant,
    marker_cleared,
    hud_transition_blocked,
    world_effects_disabled,
    world_effect_mode_initialized,
    random_result_unresolved,
    action_delay_unresolved,
};

struct WorldEffectIntroGateResult {
    std::int16_t selected_mode = -1;
    WorldEffectIntroGateAction action =
        WorldEffectIntroGateAction::marker_cleared;
};

// ROM 0x56116: enter the mode-2 intro gate or compare two thresholded rolls
// while the intro marker is negative, then perform the HUD/menu/world-mode
// follow-up in source order.
WorldEffectIntroGateResult advance_world_effect_intro_gate(
    WorldEffectModeSelectionState& state,
    const WorldEffectModeSelectionCallbacks& callbacks);

enum class WorldEffectModeSelectionAction {
    intro_gate,
    action_count,
    delayed_effect,
    record_synchronized,
    random_result_unresolved,
    action_delay_unresolved,
    secondary_record_unresolved,
};

struct WorldEffectModeSelectionResult {
    std::int16_t selected_mode = -1;
    WorldEffectModeSelectionAction action =
        WorldEffectModeSelectionAction::secondary_record_unresolved;
};

// ROM 0x561E8: synchronize the selected secondary compact runner record and
// route its status byte through the action-count, delayed-effect, or intro
// gate that supplies 0x1A370's next mode.
WorldEffectModeSelectionResult select_world_effect_animation_mode(
    WorldEffectModeSelectionState& state,
    const WorldEffectModeSelectionCallbacks& callbacks);

// ROM 0x562D4/0x562DE: add FFE169 or twice FFE16A to a source difficulty
// word. The source operates on unsigned bytes and a word accumulator.
std::uint16_t add_world_effect_timer_modifier(
    std::uint16_t base_value,
    std::uint8_t effect_timer);

std::uint16_t add_world_effect_phase_modifier(
    std::uint16_t base_value,
    std::uint8_t effect_phase);

// Host convenience for callers whose source path performs both helpers in
// sequence. The selector itself uses the exact helper-specific paths.
std::uint16_t apply_world_effect_timing_modifiers(
    std::uint16_t base_value,
    std::uint8_t effect_timer,
    std::uint8_t effect_phase);

} // namespace shadowrun::ghidra_conversion
