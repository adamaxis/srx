#include "shadowrun_ghidra/field/field_action_selection_dispatch_conversion.hpp"

namespace shadowrun::ghidra_conversion {

FieldActionSelectionRoute classify_field_action_selection(
    const FieldActionSelectionState& state) {
    if (state.menu_result < 0) {
        switch (state.saved_selection) {
        case 0:
            return {FieldActionSelectionRouteKind::selected_action_record,
                    FieldActionRecordSource::negative_selection_zero};
        case 1:
            return {FieldActionSelectionRouteKind::selected_action_record,
                    FieldActionRecordSource::negative_selection_one};
        case 2:
            return {FieldActionSelectionRouteKind::selected_action_record,
                    FieldActionRecordSource::negative_selection_two};
        case 3:
            return {FieldActionSelectionRouteKind::field_action_detail_selection,
                    std::nullopt};
        case 4:
            return {FieldActionSelectionRouteKind::runner_action_description,
                    std::nullopt};
        default:
            return {FieldActionSelectionRouteKind::remove_runner_action,
                    std::nullopt};
        }
    }

    if (state.menu_result == 0) {
        if (state.saved_selection == 3U) {
            return {FieldActionSelectionRouteKind::remove_runner_action,
                    std::nullopt};
        }
        if (state.runner_archetype_status < 2) {
            return {FieldActionSelectionRouteKind::neutral_action_unavailable,
                    std::nullopt};
        }
        if (state.saved_selection == 1U) {
            return {FieldActionSelectionRouteKind::selected_action_record,
                    FieldActionRecordSource::neutral_selection_one};
        }
        if (state.menu_mode < 3U) {
            return {FieldActionSelectionRouteKind::selected_action_record,
                    FieldActionRecordSource::neutral_low_menu_mode};
        }
        if (state.menu_mode == 3U) {
            return {FieldActionSelectionRouteKind::selected_action_record,
                    FieldActionRecordSource::neutral_menu_mode_three};
        }
        return {FieldActionSelectionRouteKind::selected_action_record,
                FieldActionRecordSource::neutral_high_menu_mode};
    }

    if (state.menu_result == 1) {
        return {FieldActionSelectionRouteKind::equipment_action, std::nullopt};
    }

    if (state.saved_selection < 3U) {
        return {FieldActionSelectionRouteKind::resource_quantity_selection,
                std::nullopt};
    }
    if (state.saved_selection == 3U) {
        return {FieldActionSelectionRouteKind::resource_record_purchase,
                std::nullopt};
    }
    if (state.saved_selection == 5U) {
        return {FieldActionSelectionRouteKind::quantity_purchase,
                std::nullopt};
    }
    return {FieldActionSelectionRouteKind::resource_purchase, std::nullopt};
}

FieldActionSelectionRoute run_field_action_selection(
    FieldActionSelectionState& state,
    const FieldActionSelectionCallbacks& callbacks) {
    state.service_result = 0;
    const auto route = classify_field_action_selection(state);
    if (callbacks.execute_route) {
        callbacks.execute_route(route, state);
    }
    return route;
}

} // namespace shadowrun::ghidra_conversion
