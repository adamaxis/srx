#pragma once

#include "shadowrun_ghidra/field/field_action_application_conversion.hpp"

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct RunnerActionRemovalState {
    FieldActionRunnerInventory runner{};
    std::uint8_t runner_slot = 0; // FFF0D0 / active record +0x66 slot
    std::uint8_t selected_runner_slot = 0; // active record +0x56
    std::uint8_t action_suffix_marker = 0; // active record +0x76
    std::uint32_t refund = 0; // FFF0BE
    std::uint32_t funds = 0; // FFFBFE
    bool menu_selection_cleared = false; // FFF098/0x1275C effect
};

enum class RunnerActionRemovalResult {
    removed,
    empty_slot,
    protected_action,
    cancelled,
};

struct RunnerActionRemovalCallbacks {
    std::function<void()> refresh_detail_panel;
    std::function<void()> show_protected_action;
    std::function<void()> render_refund;
    std::function<bool()> confirm_removal;
    std::function<void()> clear_menu_slot;
};

// ROM 0x56F4A, used by the negative/zero field-action routes for saved
// selections at or beyond the action-removal panel. It refunds the displayed
// action cost after confirmation and resets the menu slot when the removed
// runner action is the currently selected one.
RunnerActionRemovalResult remove_runner_action(
    RunnerActionRemovalState& state,
    const RunnerActionRemovalCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
