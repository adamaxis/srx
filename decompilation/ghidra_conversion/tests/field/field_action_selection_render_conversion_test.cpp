#include "shadowrun_ghidra/field/field_action_selection_render_conversion.hpp"

#include <string>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    FieldActionSelectionRenderState state{};
    state.menu_result = -1;
    state.primary_cursor = 2;

    state.saved_selection = 0;
    auto plan = plan_field_action_selection_render(state);
    if (plan.kind != FieldActionSelectionRenderKind::negative_detail ||
        !plan.detail_table ||
        plan.detail_table->family !=
            FieldActionDetailTableFamily::negative_cursor_zero) {
        return 1;
    }
    state.saved_selection = 1;
    plan = plan_field_action_selection_render(state);
    if (!plan.detail_table ||
        plan.detail_table->family !=
            FieldActionDetailTableFamily::negative_cursor_one) {
        return 2;
    }
    state.saved_selection = 2;
    plan = plan_field_action_selection_render(state);
    if (!plan.detail_table ||
        plan.detail_table->family !=
            FieldActionDetailTableFamily::negative_cursor_two) {
        return 3;
    }
    state.saved_selection = 3;
    if (plan_field_action_selection_render(state).kind !=
        FieldActionSelectionRenderKind::negative_quantity_purchase) {
        return 4;
    }
    state.primary_cursor = 6;
    if (plan_field_action_selection_render(state).kind !=
        FieldActionSelectionRenderKind::unavailable) {
        return 5;
    }
    state.saved_selection = 4;
    state.primary_cursor = 2;
    plan = plan_field_action_selection_render(state);
    if (plan.kind !=
            FieldActionSelectionRenderKind::negative_runner_action_detail ||
        !plan.suppress_runner_cost) {
        return 6;
    }
    state.saved_selection = 5;
    if (plan_field_action_selection_render(state).kind !=
        FieldActionSelectionRenderKind::negative_inventory_action_detail) {
        return 7;
    }
    state.saved_selection = static_cast<std::int8_t>(0x80U);
    state.primary_cursor = 2;
    if (plan_field_action_selection_render(state).kind !=
        FieldActionSelectionRenderKind::negative_inventory_action_detail) {
        return 19;
    }
    state.saved_selection = 3;
    state.primary_cursor = static_cast<std::int8_t>(0x80U);
    if (plan_field_action_selection_render(state).kind !=
        FieldActionSelectionRenderKind::unavailable) {
        return 20;
    }

    state.menu_result = 0;
    state.saved_selection = 0;
    state.menu_mode = 6;
    state.primary_cursor = 4;
    plan = plan_field_action_selection_render(state);
    if (plan.kind != FieldActionSelectionRenderKind::neutral_detail ||
        !plan.detail_table ||
        plan.detail_table->family !=
            FieldActionDetailTableFamily::neutral_dynamic_menu_mode ||
        plan.dynamic_table_index != 3 ||
        plan.detail_table->primary_cursor != 4) {
        return 8;
    }
    state.saved_selection = 1;
    if (plan_field_action_selection_render(state).detail_table->family !=
        FieldActionDetailTableFamily::neutral_selection_one) {
        return 9;
    }
    state.saved_selection = 2;
    if (plan_field_action_selection_render(state).detail_table->family !=
        FieldActionDetailTableFamily::neutral_low_menu) {
        return 10;
    }
    state.saved_selection = 3;
    if (plan_field_action_selection_render(state).kind !=
        FieldActionSelectionRenderKind::neutral_inventory_action_detail) {
        return 11;
    }
    state.saved_selection = static_cast<std::int8_t>(0x80U);
    if (plan_field_action_selection_render(state).detail_table->family !=
        FieldActionDetailTableFamily::neutral_low_menu) {
        return 21;
    }

    state.menu_result = 1;
    if (plan_field_action_selection_render(state).kind !=
        FieldActionSelectionRenderKind::equipment_action) {
        return 12;
    }

    state.menu_result = 2;
    state.saved_selection = 0;
    state.menu_mode = 6;
    plan = plan_field_action_selection_render(state);
    if (plan.kind != FieldActionSelectionRenderKind::resource_description ||
        !plan.resource_entry ||
        plan.resource_entry->resource.family !=
            MenuResourceFamily::menu_mode_six_records ||
        plan.resource_entry->word_offset != 8U) {
        return 13;
    }
    state.saved_selection = 3;
    plan = plan_field_action_selection_render(state);
    if (plan.kind != FieldActionSelectionRenderKind::cost_summary ||
        !plan.cost_summary ||
        plan.cost_summary->family !=
            FieldActionCostSummaryTableFamily::menu_mode_six) {
        return 14;
    }
    state.saved_selection = 5;
    state.primary_cursor = 4;
    if (plan_field_action_selection_render(state).kind !=
        FieldActionSelectionRenderKind::quantity_purchase) {
        return 15;
    }
    state.primary_cursor = 5;
    if (plan_field_action_selection_render(state).kind !=
        FieldActionSelectionRenderKind::no_action) {
        return 16;
    }
    state.saved_selection = 4;
    state.primary_cursor = 2;
    plan = plan_field_action_selection_render(state);
    if (plan.kind != FieldActionSelectionRenderKind::feedback ||
        !plan.feedback_mode || *plan.feedback_mode != 2U) {
        return 17;
    }
    state.saved_selection = static_cast<std::int8_t>(0x80U);
    plan = plan_field_action_selection_render(state);
    if (plan.kind != FieldActionSelectionRenderKind::feedback ||
        !plan.feedback_mode || *plan.feedback_mode != 2U) {
        return 22;
    }
    state.saved_selection = 5;
    state.primary_cursor = static_cast<std::int8_t>(0x80U);
    if (plan_field_action_selection_render(state).kind !=
        FieldActionSelectionRenderKind::no_action) {
        return 23;
    }

    std::vector<FieldActionGridVariant> grids;
    FieldActionSelectionRenderKind rendered_kind =
        FieldActionSelectionRenderKind::unavailable;
    state.menu_result = 2;
    state.saved_selection = 3;
    render_field_action_selection(
        state,
        FieldActionSelectionRenderCallbacks{
            [&](const FieldActionGridVariant grid) { grids.push_back(grid); },
            [&](const FieldActionSelectionRenderPlan& rendered) {
                rendered_kind = rendered.kind;
            },
        });
    if (grids.size() != 2U || grids[0] != FieldActionGridVariant::variant_b ||
        grids[1] != FieldActionGridVariant::variant_c ||
        rendered_kind != FieldActionSelectionRenderKind::cost_summary) {
        return 18;
    }

    return 0;
}
