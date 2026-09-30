#include "shadowrun_ghidra/field/field_mode_service_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

FieldModeServicePromptVariant prompt_variant(
    const std::int8_t alternate_result) {
    if (alternate_result > 0) {
        return FieldModeServicePromptVariant::positive_result_prompt;
    }
    if (alternate_result < 0) {
        return FieldModeServicePromptVariant::negative_result_prompt;
    }
    return FieldModeServicePromptVariant::default_prompt;
}

} // namespace

FieldModeServiceAction select_field_mode_service(
    FieldModeServiceState& state,
    const FieldModeServiceCallbacks& callbacks) {
    FieldModeServiceSelection selection{};
    bool matched = true;
    bool uses_service_result = false;
    bool dispatches_field_mode_service = false;
    bool writes_menu_mode = false;
    bool writes_menu_result = false;

    if (state.scene_id == 2U && state.submode == 11U) {
        selection.profile = FieldModeServiceProfile::scene_two_submode_eleven;
        selection.menu_mode = 1U;
        selection.menu_result = -1;
        writes_menu_mode = true;
        writes_menu_result = true;
        dispatches_field_mode_service = true;
    } else if (state.scene_id == 1U && state.submode == 5U) {
        selection.profile = FieldModeServiceProfile::scene_one_submode_five;
        selection.menu_mode = 2U;
        uses_service_result = true;
        writes_menu_mode = true;
    } else if (state.scene_id == 1U && state.submode == 3U) {
        selection.profile = FieldModeServiceProfile::scene_one_submode_three;
        selection.menu_mode = 4U;
        selection.menu_result = 0;
        writes_menu_mode = true;
        writes_menu_result = true;
        dispatches_field_mode_service = true;
    } else if (state.scene_id == 5U && state.submode == 2U) {
        selection.profile = FieldModeServiceProfile::scene_five_submode_two;
        selection.menu_mode = 7U;
        selection.menu_result = 1;
        writes_menu_mode = true;
        writes_menu_result = true;
        dispatches_field_mode_service = true;
    } else if (state.scene_id == 5U && state.submode == 3U) {
        selection.profile = FieldModeServiceProfile::scene_five_submode_three;
        selection.menu_mode = 5U;
        selection.menu_result = 0;
        writes_menu_mode = true;
        writes_menu_result = true;
        dispatches_field_mode_service = true;
    } else if (state.scene_id == 5U && state.submode == 4U) {
        selection.profile = FieldModeServiceProfile::scene_five_submode_four;
        selection.menu_mode = 6U;
        selection.menu_result = 2;
        writes_menu_mode = true;
        writes_menu_result = true;
        dispatches_field_mode_service = true;
    } else if (state.scene_id == 5U && state.submode == 5U) {
        selection.profile = FieldModeServiceProfile::scene_five_submode_five;
        selection.menu_mode = 0U;
        selection.menu_result = -1;
        writes_menu_mode = true;
        writes_menu_result = true;
        dispatches_field_mode_service = true;
    } else if (state.scene_id == 4U && state.submode == 3U) {
        selection.profile = FieldModeServiceProfile::scene_four_submode_three;
        uses_service_result = true;
    } else {
        matched = false;
    }

    if (!matched) {
        return FieldModeServiceAction::no_matching_route;
    }

    selection.prompt_variant = prompt_variant(state.alternate_result);
    if (writes_menu_mode) {
        state.menu_mode = selection.menu_mode;
    }
    if (writes_menu_result) {
        state.menu_result = selection.menu_result;
    }

    if (state.alternate_result != 0) {
        if (!callbacks.run_mode_exit_services) {
            return FieldModeServiceAction::mode_exit_services_unresolved;
        }
        callbacks.run_mode_exit_services(selection, state);
        state.caller_exit_status = -1;
        return FieldModeServiceAction::mode_exit_services_completed;
    }

    if (uses_service_result) {
        state.service_result = 0;
    }
    if (!callbacks.run_mode_exit_services) {
        return FieldModeServiceAction::mode_exit_services_unresolved;
    }
    callbacks.run_mode_exit_services(selection, state);

    if (uses_service_result) {
        if (state.service_result == 0) {
            state.caller_exit_status = -1;
            return FieldModeServiceAction::mode_exit_services_completed;
        }
        const auto service_result = state.service_result;
        if (state.scene_id == 1U && state.submode == 5U) {
            state.menu_result =
                service_result < 0 ? static_cast<std::int8_t>(-1)
                                   : static_cast<std::int8_t>(2);
        } else {
            // The source's (4,3) branch writes mode 3/result 0 only after a
            // nonzero FFF112 result, immediately before 0x56AC0.
            state.service_result = 0;
            state.menu_mode = 3U;
            state.menu_result = 0;
        }
    }

    if (!dispatches_field_mode_service && !uses_service_result) {
        state.caller_exit_status = -1;
        return FieldModeServiceAction::mode_exit_services_completed;
    }
    if (!callbacks.run_field_mode_service) {
        return FieldModeServiceAction::field_mode_service_unresolved;
    }
    callbacks.run_field_mode_service(state);
    state.caller_exit_status = -1;
    return FieldModeServiceAction::field_mode_service_dispatched;
}

} // namespace shadowrun::ghidra_conversion
