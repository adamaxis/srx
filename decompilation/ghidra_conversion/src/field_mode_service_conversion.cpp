#include "shadowrun_ghidra/field_mode_service_conversion.hpp"

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
    bool enters_intro_when_default = true;

    if (state.scene_id == 2U && state.submode == 11U) {
        selection.profile = FieldModeServiceProfile::scene_two_submode_eleven;
        selection.menu_mode = 1U;
        selection.menu_result = -1;
        enters_intro_when_default = true;
    } else if (state.scene_id == 1U && state.submode == 5U) {
        selection.profile = FieldModeServiceProfile::scene_one_submode_five;
        selection.menu_mode = 2U;
        uses_service_result = true;
        enters_intro_when_default = false;
    } else if (state.scene_id == 1U && state.submode == 3U) {
        selection.profile = FieldModeServiceProfile::scene_one_submode_three;
        selection.menu_mode = 4U;
        selection.menu_result = 0;
    } else if (state.scene_id == 5U && state.submode == 2U) {
        selection.profile = FieldModeServiceProfile::scene_five_submode_two;
        selection.menu_mode = 7U;
        selection.menu_result = 1;
    } else if (state.scene_id == 5U && state.submode == 3U) {
        selection.profile = FieldModeServiceProfile::scene_five_submode_three;
        selection.menu_mode = 5U;
        selection.menu_result = 0;
    } else if (state.scene_id == 5U && state.submode == 4U) {
        selection.profile = FieldModeServiceProfile::scene_five_submode_four;
        selection.menu_mode = 6U;
        selection.menu_result = 2;
    } else if (state.scene_id == 5U && state.submode == 5U) {
        selection.profile = FieldModeServiceProfile::scene_five_submode_five;
        selection.menu_mode = 0U;
        selection.menu_result = -1;
    } else if (state.scene_id == 4U && state.submode == 3U) {
        selection.profile = FieldModeServiceProfile::scene_four_submode_three;
        selection.menu_mode = 3U;
        selection.menu_result = 0;
        uses_service_result = true;
        enters_intro_when_default = false;
    } else {
        matched = false;
    }

    if (!matched) {
        return FieldModeServiceAction::no_matching_route;
    }

    selection.prompt_variant = prompt_variant(state.alternate_result);
    state.menu_mode = selection.menu_mode;
    state.menu_result = selection.menu_result;

    if (state.alternate_result != 0) {
        if (callbacks.run_mode_exit_services) {
            callbacks.run_mode_exit_services(selection, state);
        }
        return FieldModeServiceAction::service_only;
    }

    if (uses_service_result) {
        state.service_result = 0;
    }
    if (callbacks.run_mode_exit_services) {
        callbacks.run_mode_exit_services(selection, state);
    }

    if (uses_service_result) {
        if (state.service_result == 0) {
            return FieldModeServiceAction::service_only;
        }
        state.menu_result = state.service_result < 0 ? -1 : 2;
        if (state.service_result < 0) {
            return FieldModeServiceAction::service_only;
        }
    }

    if (!enters_intro_when_default && !uses_service_result) {
        return FieldModeServiceAction::service_only;
    }
    if (callbacks.run_intro_stage) {
        callbacks.run_intro_stage(state);
    }
    return FieldModeServiceAction::intro_stage_started;
}

} // namespace shadowrun::ghidra_conversion
