#include "shadowrun_ghidra/field/field_action_selection_render_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

FieldActionSelectionRenderPlan negative_result_plan(
    const FieldActionSelectionRenderState& state) {
    FieldActionSelectionRenderPlan plan{};
    plan.primary_cursor = state.primary_cursor;

    // These are byte comparisons in the source, so the signed state type is
    // intentional.  In particular, values below two select the second
    // negative-result table rather than silently becoming large unsigned
    // selections on the host.
    if (state.saved_selection == 0) {
        plan.kind = FieldActionSelectionRenderKind::negative_detail;
        plan.detail_table = FieldActionDetailTableSelection{
            FieldActionDetailTableFamily::negative_cursor_zero,
            0,
            state.primary_cursor,
        };
        return plan;
    }
    if (state.saved_selection < 2) {
        plan.kind = FieldActionSelectionRenderKind::negative_detail;
        plan.detail_table = FieldActionDetailTableSelection{
            FieldActionDetailTableFamily::negative_cursor_one,
            0,
            state.primary_cursor,
        };
        return plan;
    }
    if (state.saved_selection == 2) {
        plan.kind = FieldActionSelectionRenderKind::negative_detail;
        plan.detail_table = FieldActionDetailTableSelection{
            FieldActionDetailTableFamily::negative_cursor_two,
            0,
            state.primary_cursor,
        };
        return plan;
    }
    if (state.saved_selection < 4) {
        // 0x585FA exits through the unavailable text path when the primary
        // cursor is six or higher; otherwise it renders a quantity offer.
        plan.kind = state.primary_cursor >= 6
                        ? FieldActionSelectionRenderKind::unavailable
                        : FieldActionSelectionRenderKind::negative_quantity_purchase;
        return plan;
    }
    if (state.saved_selection == 4) {
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
    plan.primary_cursor = state.primary_cursor;

    if (state.saved_selection == 3) {
        plan.kind = FieldActionSelectionRenderKind::neutral_inventory_action_detail;
        return plan;
    }

    plan.kind = FieldActionSelectionRenderKind::neutral_detail;
    if (state.saved_selection == 1) {
        plan.detail_table = FieldActionDetailTableSelection{
            FieldActionDetailTableFamily::neutral_selection_one,
            0,
            state.primary_cursor,
        };
    } else if (state.saved_selection < 1) {
        plan.dynamic_table_index =
            static_cast<std::int16_t>(state.menu_mode) - 3;
        plan.detail_table = FieldActionDetailTableSelection{
            FieldActionDetailTableFamily::neutral_dynamic_menu_mode,
            plan.dynamic_table_index,
            state.primary_cursor,
        };
    } else {
        plan.detail_table = FieldActionDetailTableSelection{
            FieldActionDetailTableFamily::neutral_low_menu,
            0,
            state.primary_cursor,
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
    plan.primary_cursor = state.primary_cursor;
    if (state.menu_result == 1) {
        plan.kind = FieldActionSelectionRenderKind::equipment_action;
        return plan;
    }

    // 0x57C7A compares the saved selection as a signed byte before entering
    // the resource-description branch.  The normal game values are 0..5,
    // but preserving the source predicate keeps malformed extracted state
    // visible instead of silently routing it to a different leaf.
    if (state.saved_selection < 3) {
        plan.kind = FieldActionSelectionRenderKind::resource_description;
        plan.resource_entry = select_menu_resource_entry(
            state.menu_mode, state.saved_selection, state.primary_cursor);
        return plan;
    }
    if (state.saved_selection == 3) {
        plan.kind = FieldActionSelectionRenderKind::cost_summary;
        plan.cost_summary = FieldActionCostSummarySelection{
            state.menu_mode == 6
                ? FieldActionCostSummaryTableFamily::menu_mode_six
                : FieldActionCostSummaryTableFamily::other_menu_modes,
            state.primary_cursor,
        };
        return plan;
    }
    if (state.saved_selection == 5) {
        plan.kind = state.primary_cursor >= 5
                        ? FieldActionSelectionRenderKind::unavailable
                        : FieldActionSelectionRenderKind::quantity_purchase;
        return plan;
    }

    plan.kind = FieldActionSelectionRenderKind::feedback;
    plan.feedback_mode = static_cast<std::uint8_t>(state.primary_cursor);
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
