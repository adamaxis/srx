#pragma once

#include "shadowrun_ghidra/field/field_action_application_conversion.hpp"

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct FieldActionDetailPurchaseState {
    FieldActionRunnerInventory runner{};
    std::uint8_t runner_slot = 0; // FFF0C2 / active record +0x6E slot
    std::uint8_t category = 0; // FFF0D0
    std::uint32_t cost = 0; // FFF0BE
    std::uint32_t funds = 0; // FFFBFE
};

enum class FieldActionDetailPurchaseResult {
    applied,
    invalid_selection,
    already_owned,
    incompatible_action,
    insufficient_funds,
    cancelled,
};

struct FieldActionDetailPurchaseCallbacks {
    // ROM 0x57C66 draws the action-detail grid before the state checks.
    std::function<void()> render_detail_grid;
    std::function<void()> show_already_owned;
    std::function<void()> show_incompatible_action;
    std::function<void()> show_insufficient_funds;
    std::function<bool()> confirm_purchase;
};

// Negative-result saved-selection-3 branch at ROM 0x57026. It checks the
// selected runner action's category bit, applies the source action-code
// compatibility rule, confirms and charges FFF0BE, then sets the category bit.
// Category zero additionally clears companion bit three exactly as the ROM
// does after setting bit zero.
FieldActionDetailPurchaseResult apply_field_action_detail_purchase(
    FieldActionDetailPurchaseState& state,
    const FieldActionDetailPurchaseCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
