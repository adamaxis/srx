#include "shadowrun_ghidra/field/field_action_selection_dispatch_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

bool runner_action_progression_is_blocked(const std::int8_t status) {
    // The source evaluates BMI/negative-byte arithmetic as
    // -1 < (char)(status - 2). Preserve the byte wrap instead of reducing
    // this to a host unsigned comparison.
    const auto delta = static_cast<std::int8_t>(static_cast<std::uint8_t>(
        static_cast<std::uint8_t>(status) - 2U));
    return delta >= 0;
}

} // namespace

FieldActionSelectionRoute classify_field_action_selection(
    const FieldActionSelectionState& state) {
    if (state.menu_result < 0) {
        switch (state.selected_action) {
        case 0:
            return {FieldActionSelectionRouteKind::apply_selected_field_action,
                    FieldActionRecordSource::negative_selection_zero};
        case 1:
            return {FieldActionSelectionRouteKind::apply_selected_field_action,
                    FieldActionRecordSource::negative_selection_one};
        case 2:
            return {FieldActionSelectionRouteKind::apply_selected_field_action,
                    FieldActionRecordSource::negative_selection_two};
        case 3:
            return {FieldActionSelectionRouteKind::purchase_field_action_detail,
                    std::nullopt};
        case 4:
            return {FieldActionSelectionRouteKind::show_runner_action_description,
                    std::nullopt};
        default:
            return {FieldActionSelectionRouteKind::remove_runner_action,
                    std::nullopt};
        }
    }

    if (state.menu_result == 0) {
        if (state.selected_action == 3U) {
            return {FieldActionSelectionRouteKind::remove_runner_action,
                    std::nullopt};
        }
        if (runner_action_progression_is_blocked(
                state.runner_archetype_status)) {
            return {FieldActionSelectionRouteKind::runner_action_unavailable,
                    std::nullopt};
        }
        if (state.selected_action == 1U) {
            return {FieldActionSelectionRouteKind::apply_selected_field_action,
                    FieldActionRecordSource::neutral_selection_one};
        }
        if (state.menu_mode < 3U) {
            return {FieldActionSelectionRouteKind::apply_selected_field_action,
                    FieldActionRecordSource::neutral_low_menu_mode};
        }
        if (state.menu_mode == 3U) {
            return {FieldActionSelectionRouteKind::apply_selected_field_action,
                    FieldActionRecordSource::neutral_menu_mode_three};
        }
        return {FieldActionSelectionRouteKind::apply_selected_field_action,
                FieldActionRecordSource::neutral_high_menu_mode};
    }

    if (state.menu_result == 1) {
        return {FieldActionSelectionRouteKind::apply_field_equipment_upgrade,
                std::nullopt};
    }

    if (state.selected_action < 3U) {
        return {FieldActionSelectionRouteKind::install_matrix_program_resource,
                std::nullopt};
    }
    if (state.selected_action == 3U) {
        return {FieldActionSelectionRouteKind::purchase_field_resource_record,
                std::nullopt};
    }
    if (state.selected_action == 5U) {
        return {FieldActionSelectionRouteKind::purchase_field_quantity,
                std::nullopt};
    }
    return {FieldActionSelectionRouteKind::purchase_field_resource,
            std::nullopt};
}

FieldActionSelectionRoute run_field_action_selection(
    FieldActionSelectionState& state,
    const FieldActionSelectionCallbacks& callbacks) {
    state.service_result = 0;
    const auto route = classify_field_action_selection(state);
    switch (route.kind) {
    case FieldActionSelectionRouteKind::apply_selected_field_action:
        if (callbacks.apply_selected_field_action && route.record_source) {
            callbacks.apply_selected_field_action(*route.record_source, state);
        }
        break;
    case FieldActionSelectionRouteKind::purchase_field_action_detail:
        if (callbacks.purchase_field_action_detail) {
            callbacks.purchase_field_action_detail(state);
        }
        break;
    case FieldActionSelectionRouteKind::show_runner_action_description:
        if (callbacks.show_runner_action_description) {
            callbacks.show_runner_action_description(state);
        }
        break;
    case FieldActionSelectionRouteKind::remove_runner_action:
        if (callbacks.remove_runner_action) {
            callbacks.remove_runner_action(state);
        }
        break;
    case FieldActionSelectionRouteKind::runner_action_unavailable:
        if (callbacks.runner_action_unavailable) {
            callbacks.runner_action_unavailable(state);
        }
        break;
    case FieldActionSelectionRouteKind::install_matrix_program_resource:
        if (callbacks.install_matrix_program_resource) {
            callbacks.install_matrix_program_resource(state);
        }
        break;
    case FieldActionSelectionRouteKind::purchase_field_resource_record:
        if (callbacks.purchase_field_resource_record) {
            callbacks.purchase_field_resource_record(state);
        }
        break;
    case FieldActionSelectionRouteKind::purchase_field_quantity:
        if (callbacks.purchase_field_quantity) {
            callbacks.purchase_field_quantity(state);
        }
        break;
    case FieldActionSelectionRouteKind::purchase_field_resource:
        if (callbacks.purchase_field_resource) {
            callbacks.purchase_field_resource(state);
        }
        break;
    case FieldActionSelectionRouteKind::apply_field_equipment_upgrade:
        if (callbacks.apply_field_equipment_upgrade) {
            callbacks.apply_field_equipment_upgrade(state);
        }
        break;
    }
    return route;
}

} // namespace shadowrun::ghidra_conversion
