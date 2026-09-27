#include "shadowrun_ghidra/field/field_action_selection_dispatch_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldActionSelectionState state{};
    state.menu_result = -1;
    state.saved_selection = 0U;
    auto route = classify_field_action_selection(state);
    if (!expect(route.kind == FieldActionSelectionRouteKind::apply_selected_field_action &&
                route.record_source ==
                    FieldActionRecordSource::negative_selection_zero)) {
        return 1;
    }

    state.saved_selection = 1U;
    if (!expect(classify_field_action_selection(state).record_source ==
                FieldActionRecordSource::negative_selection_one)) {
        return 2;
    }
    state.saved_selection = 2U;
    if (!expect(classify_field_action_selection(state).record_source ==
                FieldActionRecordSource::negative_selection_two)) {
        return 3;
    }
    state.saved_selection = 3U;
    if (!expect(classify_field_action_selection(state).kind ==
                FieldActionSelectionRouteKind::purchase_field_action_detail)) {
        return 4;
    }
    state.saved_selection = 4U;
    if (!expect(classify_field_action_selection(state).kind ==
                FieldActionSelectionRouteKind::show_runner_action_description)) {
        return 5;
    }
    state.saved_selection = 7U;
    if (!expect(classify_field_action_selection(state).kind ==
                FieldActionSelectionRouteKind::remove_runner_action)) {
        return 6;
    }

    state.menu_result = 0;
    state.saved_selection = 0U;
    state.runner_archetype_status = 2;
    if (!expect(classify_field_action_selection(state).kind ==
                FieldActionSelectionRouteKind::runner_action_unavailable)) {
        return 7;
    }
    state.runner_archetype_status = 1;
    state.saved_selection = 1U;
    if (!expect(classify_field_action_selection(state).record_source ==
                FieldActionRecordSource::neutral_selection_one)) {
        return 8;
    }
    state.saved_selection = 0U;
    state.menu_mode = 2U;
    if (!expect(classify_field_action_selection(state).record_source ==
                FieldActionRecordSource::neutral_low_menu_mode)) {
        return 9;
    }
    state.menu_mode = 3U;
    if (!expect(classify_field_action_selection(state).record_source ==
                FieldActionRecordSource::neutral_menu_mode_three)) {
        return 10;
    }
    state.menu_mode = 4U;
    if (!expect(classify_field_action_selection(state).record_source ==
                FieldActionRecordSource::neutral_high_menu_mode)) {
        return 11;
    }
    state.saved_selection = 3U;
    if (!expect(classify_field_action_selection(state).kind ==
                FieldActionSelectionRouteKind::remove_runner_action)) {
        return 12;
    }

    state.menu_result = 1;
    if (!expect(classify_field_action_selection(state).kind ==
                FieldActionSelectionRouteKind::apply_field_equipment_upgrade)) {
        return 13;
    }

    state.menu_result = 2;
    state.saved_selection = 0U;
    if (!expect(classify_field_action_selection(state).kind ==
                FieldActionSelectionRouteKind::install_matrix_program_resource)) {
        return 14;
    }
    state.saved_selection = 3U;
    if (!expect(classify_field_action_selection(state).kind ==
                FieldActionSelectionRouteKind::purchase_field_resource_record)) {
        return 15;
    }
    state.saved_selection = 5U;
    if (!expect(classify_field_action_selection(state).kind ==
                FieldActionSelectionRouteKind::purchase_field_quantity)) {
        return 16;
    }
    state.saved_selection = 4U;
    if (!expect(classify_field_action_selection(state).kind ==
                FieldActionSelectionRouteKind::purchase_field_resource)) {
        return 17;
    }

    std::vector<FieldActionSelectionRouteKind> executed;
    std::vector<FieldActionRecordSource> sources;
    state.menu_result = -1;
    state.saved_selection = 2U;
    const auto executed_route = run_field_action_selection(
        state,
        FieldActionSelectionCallbacks{
            [&](const FieldActionRecordSource source,
                FieldActionSelectionState& callback_state) {
                executed.push_back(
                    FieldActionSelectionRouteKind::apply_selected_field_action);
                sources.push_back(source);
                callback_state.service_result = -1;
            },
        });
    if (!expect(executed_route.kind ==
                    FieldActionSelectionRouteKind::apply_selected_field_action &&
                state.service_result == -1 && executed.size() == 1U &&
                sources.size() == 1U &&
                sources.front() == FieldActionRecordSource::negative_selection_two &&
                executed.front() ==
                    FieldActionSelectionRouteKind::apply_selected_field_action)) {
        return 18;
    }

    return 0;
}
