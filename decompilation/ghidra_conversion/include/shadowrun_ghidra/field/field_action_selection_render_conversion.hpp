#pragma once

#include "shadowrun_ghidra/field/field_action_grid_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_selection_presentation_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

// The detail-record tables selected by the presentation dispatcher are
// extracted data families.  They are deliberately named by the source
// selection that reaches them, rather than by their original ROM addresses.
enum class FieldActionDetailTableFamily {
    negative_cursor_zero,
    negative_cursor_one,
    negative_cursor_two,
    neutral_dynamic_menu_mode,
    neutral_selection_one,
    neutral_low_menu,
};

struct FieldActionDetailTableSelection {
    FieldActionDetailTableFamily family =
        FieldActionDetailTableFamily::negative_cursor_zero;
    std::int16_t menu_mode_delta = 0;
    std::int8_t action_cursor = 0;
};

enum class FieldActionCostSummaryTableFamily {
    menu_mode_six,
    other_menu_modes,
};

struct FieldActionCostSummarySelection {
    FieldActionCostSummaryTableFamily family =
        FieldActionCostSummaryTableFamily::other_menu_modes;
    std::int8_t action_cursor = 0;
};

enum class FieldActionSelectionRenderKind {
    negative_detail,
    negative_quantity_purchase,
    negative_runner_action_detail,
    negative_inventory_action_detail,
    neutral_detail,
    neutral_inventory_action_detail,
    equipment_action,
    resource_description,
    cost_summary,
    quantity_purchase,
    no_action,
    unavailable,
    feedback,
};

struct FieldActionSelectionRenderState {
    std::uint8_t menu_mode = 0; // FFF0B6
    std::int8_t menu_result = 0; // FFF0B7
    std::int8_t selected_action = 0; // FFF0B8
    std::int8_t action_cursor = 0; // FFF0D0
};

struct FieldActionSelectionRenderPlan {
    // 0x57C7A always writes these two panels before dispatching the selected
    // branch. The array makes the source order explicit and keeps the host
    // presentation loop uniform, including the source's positive
    // selection-5/no-action cursor cutoff, including the source byte flags.
    std::array<FieldActionGridVariant, 2> prelude_grids{
        FieldActionGridVariant::variant_b,
        FieldActionGridVariant::variant_c,
    };

    FieldActionSelectionRenderKind kind =
        FieldActionSelectionRenderKind::unavailable;
    std::optional<FieldActionDetailTableSelection> detail_table;
    std::optional<MenuResourceEntry> resource_entry;
    std::optional<FieldActionCostSummarySelection> cost_summary;
    std::optional<std::uint8_t> feedback_mode;
    std::int8_t action_cursor = 0;
    std::int16_t dynamic_table_index = 0;
    bool suppress_runner_cost = false;
};

struct FieldActionSelectionRenderCallbacks {
    std::function<void(FieldActionGridVariant)> write_prelude_grid;
    // The selected leaf owns the detailed typed data and presentation calls.
    // This callback receives the source-derived orchestration plan only.
    std::function<void(const FieldActionSelectionRenderPlan&)> render_branch;
};

// ROM 0x57C7A. Select the field-action presentation branch after the source
// has already established menu mode/result, saved selection, and cursor.
// This function only selects extracted data families and leaf boundaries; it
// does not retain ROM addresses or emulate the Genesis display hardware.
FieldActionSelectionRenderPlan plan_field_action_selection_render(
    const FieldActionSelectionRenderState& state);

// Execute the fixed grid prelude in source order, then hand the selected
// typed branch to its existing conversion leaf.
FieldActionSelectionRenderPlan render_field_action_selection(
    const FieldActionSelectionRenderState& state,
    const FieldActionSelectionRenderCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
