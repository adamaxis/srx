#include "shadowrun_ghidra/field/field_action_selection_render_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

bool source_cmp_byte_is_negative(const std::int8_t value,
                                 const std::uint8_t immediate) {
    // The source uses CMP.B #immediate,value followed by BMI/BPL.  BMI/BPL
    // inspect the wrapped subtraction's N bit; a signed C++ ordering is not
    // equivalent for high-bit source bytes.
    return (static_cast<std::uint8_t>(value) - immediate) & 0x80U;
}

FieldActionSelectionRenderPlan negative_result_plan(
    const FieldActionSelectionRenderState& state) {
    FieldActionSelectionRenderPlan plan{};
    plan.action_cursor = state.action_cursor;

    // The source uses CMP.B followed by BMI at each saved-selection boundary;
    // the conversion preserves those wrapped N-bit decisions rather than
    // widening source bytes into host signed comparisons.
    if (state.selected_action == 0) {
        plan.kind = FieldActionSelectionRenderKind::negative_detail;
        plan.detail_table = FieldActionDetailTableSelection{
            FieldActionDetailTableFamily::negative_cursor_zero,
            0,
            state.action_cursor,
        };
        return plan;
    }
    if (source_cmp_byte_is_negative(state.selected_action, 2U)) {
        plan.kind = FieldActionSelectionRenderKind::negative_detail;
        plan.detail_table = FieldActionDetailTableSelection{
            FieldActionDetailTableFamily::negative_cursor_one,
            0,
            state.action_cursor,
        };
        return plan;
    }
    if (state.selected_action == 2) {
        plan.kind = FieldActionSelectionRenderKind::negative_detail;
        plan.detail_table = FieldActionDetailTableSelection{
            FieldActionDetailTableFamily::negative_cursor_two,
            0,
            state.action_cursor,
        };
        return plan;
    }
    if (source_cmp_byte_is_negative(state.selected_action, 4U)) {
        // 0x585FA exits through the unavailable text path when the primary
        // cursor is six or higher; otherwise it renders a quantity offer.
        plan.kind = source_cmp_byte_is_negative(state.action_cursor, 6U)
                        ? FieldActionSelectionRenderKind::negative_quantity_purchase
                        : FieldActionSelectionRenderKind::unavailable;
        return plan;
    }
    if (state.selected_action == 4) {
        plan.kind = FieldActionSelectionRenderKind::negative_runner_action_detail;
        plan.suppress_runner_cost = true; // 0x58540 sets FFF0B1 to -1.
        return plan;
    }

    plan.kind = FieldActionSelectionRenderKind::negative_inventory_action_detail;
    return plan;
}

FieldActionSelectionRenderPlan neutral_result_plan(
    const FieldActionSelectionRenderState& state) {
    FieldActionSelectionRenderPlan plan{};
    plan.action_cursor = state.action_cursor;

    if (state.selected_action == 3) {
        plan.kind = FieldActionSelectionRenderKind::neutral_inventory_action_detail;
        return plan;
    }

    plan.kind = FieldActionSelectionRenderKind::neutral_detail;
    if (state.selected_action == 1) {
        plan.detail_table = FieldActionDetailTableSelection{
            FieldActionDetailTableFamily::neutral_selection_one,
            0,
            state.action_cursor,
        };
    } else if (source_cmp_byte_is_negative(state.selected_action, 1U)) {
        plan.dynamic_table_index =
            static_cast<std::int16_t>(state.menu_mode) - 3;
        plan.detail_table = FieldActionDetailTableSelection{
            FieldActionDetailTableFamily::neutral_dynamic_menu_mode,
            plan.dynamic_table_index,
            state.action_cursor,
        };
    } else {
        plan.detail_table = FieldActionDetailTableSelection{
            FieldActionDetailTableFamily::neutral_low_menu,
            0,
            state.action_cursor,
        };
    }
    return plan;
}

} // namespace

FieldActionSelectionRenderPlan plan_field_action_selection_render(
    const FieldActionSelectionRenderState& state) {
    if (state.menu_result < 0) {
        return negative_result_plan(state);
    }

    if (state.menu_result == 0) {
        return neutral_result_plan(state);
    }

    FieldActionSelectionRenderPlan plan{};
    plan.action_cursor = state.action_cursor;
    if (state.menu_result == 1) {
        plan.kind = FieldActionSelectionRenderKind::equipment_action;
        return plan;
    }

    // 0x57C7A compares the saved selection as a signed byte before entering
    // the resource-description branch.  The normal game values are 0..5,
    // but preserving the source predicate keeps malformed extracted state
    // visible instead of silently routing it to a different leaf.
    if (source_cmp_byte_is_negative(state.selected_action, 3U)) {
        plan.kind = FieldActionSelectionRenderKind::resource_description;
        plan.resource_entry = select_menu_resource_entry(
            state.menu_mode, state.selected_action, state.action_cursor);
        return plan;
    }
    if (state.selected_action == 3) {
        plan.kind = FieldActionSelectionRenderKind::cost_summary;
        plan.cost_summary = FieldActionCostSummarySelection{
            state.menu_mode == 6
                ? FieldActionCostSummaryTableFamily::menu_mode_six
                : FieldActionCostSummaryTableFamily::other_menu_modes,
            state.action_cursor,
        };
        return plan;
    }
    if (state.selected_action == 5) {
        plan.kind = !source_cmp_byte_is_negative(state.action_cursor, 5U)
                        ? FieldActionSelectionRenderKind::no_action
                        : FieldActionSelectionRenderKind::quantity_purchase;
        return plan;
    }

    plan.kind = FieldActionSelectionRenderKind::feedback;
    plan.feedback_mode = static_cast<std::uint8_t>(state.action_cursor);
    return plan;
}

FieldActionSelectionRenderPlan render_field_action_selection(
    const FieldActionSelectionRenderState& state,
    const FieldActionSelectionRenderCallbacks& callbacks) {
    const auto plan = plan_field_action_selection_render(state);
    if (callbacks.write_prelude_grid) {
        for (const auto grid : plan.prelude_grids) {
            callbacks.write_prelude_grid(grid);
        }
    }
    if (callbacks.render_branch) {
        callbacks.render_branch(plan);
    }
    return plan;
}

} // namespace shadowrun::ghidra_conversion
