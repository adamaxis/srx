#pragma once

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

// The second action-resolution continuation reached from the title/world
// transition.  ROM 0x56050 uses the selected compact runner record to choose
// a trial count, rolls against FFFBBE+0x0A, and subtracts that roll count from
// the live runner action result byte.
struct TitleWorldActionMarkerState {
    std::uint8_t intro_threshold = 0; // FFFBBE + 0x0A
    std::uint8_t active_runner_action_result = 0; // live record +0x9F
    std::uint8_t thresholded_rolls = 0; // FFF0DD
};

struct TitleWorldActionMarkerCallbacks {
    // ROM 0x19BF6 produces D6 from the selected compact record's auxiliary
    // value and the current mode variant.
    std::function<std::optional<std::uint16_t>()>
        select_auxiliary_trial_count;
    // ROM 0x0DBA receives D6 as trials and D4 as the difficulty, then writes
    // the qualifying-roll count to FFF0DD.
    std::function<std::optional<std::uint8_t>(
        std::uint16_t trial_count, std::uint16_t difficulty)>
        count_thresholded_random_rolls;
};

enum class TitleWorldActionMarkerOutcome {
    resolved,
    dependency_unavailable,
};

struct TitleWorldActionMarkerResult {
    TitleWorldActionMarkerOutcome outcome =
        TitleWorldActionMarkerOutcome::dependency_unavailable;
    // D4 after the source SUB.B.  0x1A094 publishes this raw byte to the
    // pending-action marker even when the live result is subsequently cleared.
    std::uint8_t pending_marker = 0;
    bool active_result_cleared = false;
};

// ROM 0x56050: resolve the title/world transition's second runner-action
// roll.  The source clamps the difficulty to two, stores the signed-byte
// result in the live action record, clears that record on BPL failure, and
// leaves the raw result available for the caller's FFF0AA publication.
TitleWorldActionMarkerResult resolve_title_world_action_marker(
    TitleWorldActionMarkerState& state,
    const TitleWorldActionMarkerCallbacks& callbacks);

struct TitleWorldActionSelectionState {
    bool world_effects_active = false; // FFDF84 != 0
    std::uint8_t pending_action_marker = 0; // FFF0AA
};

struct TitleWorldActionSelectionCallbacks {
    // ROM 0x19BAE returns the signed byte selected from the active runner's
    // action/status table.  The signed result is retained across 0x55FD8.
    std::function<std::optional<std::int8_t>()>
        select_active_runner_hud_mode;
    // ROM 0x55FD8 calculates and publishes the first action-result roll.  Its
    // source D0 is preserved, so the caller continues using the selected HUD
    // mode after this callback returns.
    std::function<void()> calculate_and_publish_runner_action_result;
    // ROM 0x56050 returns the second-roll marker (D4) used by 0x1A094.
    std::function<std::optional<std::uint8_t>()>
        resolve_title_world_action_marker;
    // ROM 0x17C84 / 0x17C52 / 0x1B706.  The exact publication order is part
    // of both the active-world and HUD-mode 1/8 branches.
    std::function<void(std::uint8_t source_code, std::uint8_t value)>
        set_hud_selection;
    std::function<void()> render_menu_text_followup;
    std::function<void()> prepare_menu_offset_once;
};

enum class TitleWorldActionSelectionAction {
    selected_world_mode,
    action_roll_rejected,
    action_marker_resolved,
    hud_mode_unavailable,
    action_result_unavailable,
    action_marker_unavailable,
};

struct TitleWorldActionSelectionResult {
    TitleWorldActionSelectionAction action =
        TitleWorldActionSelectionAction::hud_mode_unavailable;
    std::optional<std::int16_t> return_code;
    std::optional<std::int8_t> selected_hud_mode;
};

// ROM 0x1A030: service the title/world transition's runner-action gate.  A
// nonzero FFDF84 publishes code 9/value 0x0B; selected HUD mode 1/8 publishes
// code 9/value 0x0C.  Both enter menu-offset preparation.  Other nonnegative
// modes run the two action-result continuations; a negative mode prepares the
// offset and returns -1.
TitleWorldActionSelectionResult resolve_title_world_action_selection(
    TitleWorldActionSelectionState& state,
    const TitleWorldActionSelectionCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
