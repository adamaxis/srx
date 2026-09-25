#pragma once

#include "shadowrun_ghidra/field_action_cost_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

// The active runner record exposes eight ordinary action/inventory slots at
// +0x66..+0x6D and eight companion values at +0x6E..+0x75.  The host model
// keeps those fields together instead of exposing the source record address.
struct FieldActionRunnerInventory {
    std::array<std::uint8_t, 8> action_codes{};
    std::array<std::uint8_t, 8> companion_values{};
    std::uint8_t special_action_count = 0; // source active record +0x4F
};

struct FieldActionPurchaseState {
    FieldActionRunnerInventory runner{};
    std::uint32_t funds = 0; // source FFFBFE
};

enum class FieldActionApplicationResult {
    applied,
    invalid_record,
    no_inventory_slot,
    insufficient_funds,
    cancelled,
    special_action_limit,
};

struct FieldActionApplicationCallbacks {
    // 0x57C5E refreshes the action detail panel before cost handling.
    std::function<void()> refresh_action_detail_panel;
    std::function<void()> show_no_inventory_slot;
    std::function<void()> show_insufficient_funds;
    std::function<void()> show_special_action_limit;

    // 0x57B8C is an interactive quantity/cost chooser.  Returning no value
    // is the source cancel path.  An absent callback accepts the preview cost
    // so the behavior can be used by a non-interactive host controller.
    std::function<std::optional<std::uint32_t>(std::uint32_t preview_cost)>
        choose_purchase_cost;
};

// ROM 0x5794E.  Apply one selected field-menu action to the active runner.
// The base-rate lookup and cost context are the same typed inputs used by the
// neighboring 0x57A44/0x57A6E conversion.
FieldActionApplicationResult apply_selected_field_action(
    const FieldActionCostRecord& record,
    const FieldActionCostContext& cost_context,
    FieldActionPurchaseState& state,
    const FieldActionBaseCostLookup& base_cost_lookup,
    const FieldActionApplicationCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
