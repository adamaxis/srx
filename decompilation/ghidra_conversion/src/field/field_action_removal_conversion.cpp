#include "shadowrun_ghidra/field/field_action_removal_conversion.hpp"

namespace shadowrun::ghidra_conversion {

RunnerActionRemovalResult remove_runner_action(
    RunnerActionRemovalState& state,
    const RunnerActionRemovalCallbacks& callbacks) {
    if (callbacks.refresh_detail_panel) {
        callbacks.refresh_detail_panel();
    }

    if (state.runner_slot >= state.runner.action_codes.size()) {
        return RunnerActionRemovalResult::empty_slot;
    }
    const auto action_code = state.runner.action_codes[state.runner_slot];
    if (action_code == 0U) {
        return RunnerActionRemovalResult::empty_slot;
    }
    if (action_code == 0x29U || action_code == 0x2AU ||
        action_code == 0x2BU) {
        if (callbacks.show_protected_action) {
            callbacks.show_protected_action();
        }
        return RunnerActionRemovalResult::protected_action;
    }

    if (callbacks.render_refund) {
        callbacks.render_refund();
    }
    if (callbacks.confirm_removal && !callbacks.confirm_removal()) {
        return RunnerActionRemovalResult::cancelled;
    }

    state.funds += state.refund;
    if (action_code ==
        static_cast<std::uint8_t>(state.action_suffix_marker + 0x2BU)) {
        state.action_suffix_marker = 0U;
    }
    if (state.runner_slot == state.selected_runner_slot) {
        state.menu_selection_cleared = true;
        if (callbacks.clear_menu_slot) {
            callbacks.clear_menu_slot();
        }
    }
    state.runner.action_codes[state.runner_slot] = 0U;
    state.runner.companion_values[state.runner_slot] = 0U;
    return RunnerActionRemovalResult::removed;
}

} // namespace shadowrun::ghidra_conversion
