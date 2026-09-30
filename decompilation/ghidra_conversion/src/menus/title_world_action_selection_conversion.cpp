#include "shadowrun_ghidra/menus/title_world_action_selection_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

std::uint8_t subtract_byte(const std::uint8_t minuend,
                           const std::uint8_t subtrahend) {
    return static_cast<std::uint8_t>(minuend - subtrahend);
}

void publish_selection(
    const TitleWorldActionSelectionCallbacks& callbacks,
    const std::uint8_t value) {
    if (callbacks.set_hud_selection) {
        callbacks.set_hud_selection(9U, value);
    }
    if (callbacks.render_menu_text_followup) {
        callbacks.render_menu_text_followup();
    }
    if (callbacks.prepare_menu_offset_once) {
        callbacks.prepare_menu_offset_once();
    }
}

} // namespace

TitleWorldActionMarkerResult resolve_title_world_action_marker(
    TitleWorldActionMarkerState& state,
    const TitleWorldActionMarkerCallbacks& callbacks) {
    if (!callbacks.select_auxiliary_trial_count ||
        !callbacks.count_thresholded_random_rolls) {
        return {};
    }

    const auto trial_count = callbacks.select_auxiliary_trial_count();
    if (!trial_count.has_value()) {
        return {};
    }

    const auto difficulty = state.intro_threshold < 2U
                               ? static_cast<std::uint16_t>(2U)
                               : static_cast<std::uint16_t>(
                                     state.intro_threshold);
    const auto rolls = callbacks.count_thresholded_random_rolls(
        *trial_count, difficulty);
    if (!rolls.has_value()) {
        return {};
    }

    state.thresholded_rolls = *rolls;
    const auto raw_result = subtract_byte(
        state.active_runner_action_result, state.thresholded_rolls);
    const bool cleared = (raw_result & 0x80U) != 0U;
    // 0x560A2 is BPL, not BLE: only the result's sign bit controls this
    // clear, and the raw D4 byte remains available to 0x1A094.
    state.active_runner_action_result = cleared ? 0U : raw_result;
    return {TitleWorldActionMarkerOutcome::resolved, raw_result, cleared};
}

TitleWorldActionSelectionResult resolve_title_world_action_selection(
    TitleWorldActionSelectionState& state,
    const TitleWorldActionSelectionCallbacks& callbacks) {
    TitleWorldActionSelectionResult result{};

    if (state.world_effects_active) {
        publish_selection(callbacks, 0x0BU);
        result.action = TitleWorldActionSelectionAction::selected_world_mode;
        result.return_code = 9;
        return result;
    }

    if (!callbacks.select_active_runner_hud_mode) {
        result.action = TitleWorldActionSelectionAction::hud_mode_unavailable;
        return result;
    }
    const auto selected_mode = callbacks.select_active_runner_hud_mode();
    if (!selected_mode.has_value()) {
        result.action = TitleWorldActionSelectionAction::hud_mode_unavailable;
        return result;
    }
    result.selected_hud_mode = *selected_mode;

    if (*selected_mode == 1 || *selected_mode == 8) {
        publish_selection(callbacks, 0x0CU);
        result.action = TitleWorldActionSelectionAction::selected_world_mode;
        result.return_code = 9;
        return result;
    }

    if (!callbacks.calculate_and_publish_runner_action_result) {
        result.action =
            TitleWorldActionSelectionAction::action_result_unavailable;
        return result;
    }
    callbacks.calculate_and_publish_runner_action_result();

    if (*selected_mode < 0) {
        if (callbacks.prepare_menu_offset_once) {
            callbacks.prepare_menu_offset_once();
        }
        result.action = TitleWorldActionSelectionAction::action_roll_rejected;
        result.return_code = -1;
        return result;
    }

    if (!callbacks.resolve_title_world_action_marker) {
        result.action =
            TitleWorldActionSelectionAction::action_marker_unavailable;
        return result;
    }
    const auto marker = callbacks.resolve_title_world_action_marker();
    if (!marker.has_value()) {
        result.action =
            TitleWorldActionSelectionAction::action_marker_unavailable;
        return result;
    }
    state.pending_action_marker = *marker;
    result.action = TitleWorldActionSelectionAction::action_marker_resolved;
    result.return_code = *selected_mode;
    return result;
}

} // namespace shadowrun::ghidra_conversion
