#include "shadowrun_ghidra/field/field_mode_service_conversion.hpp"

#include <cstdint>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldModeServiceState state{2U, 11U, 0U, 0, 0, 0};
    FieldModeServiceSelection observed{};
    int services = 0;
    int field_mode_services = 0;
    std::int8_t callback_service_result = 0;
    const FieldModeServiceCallbacks callbacks{
        [&](FieldModeServiceSelection& selection, FieldModeServiceState& callback_state) {
            observed = selection;
            ++services;
            if (selection.profile ==
                    FieldModeServiceProfile::scene_one_submode_five ||
                selection.profile ==
                    FieldModeServiceProfile::scene_four_submode_three) {
                callback_state.service_result = callback_service_result;
            }
        },
        [&](FieldModeServiceState&) { ++field_mode_services; }};

    if (expect(select_field_mode_service(state, callbacks) ==
                   FieldModeServiceAction::field_mode_service_dispatched &&
               observed.profile ==
                   FieldModeServiceProfile::scene_two_submode_eleven &&
               observed.prompt_variant ==
                   FieldModeServicePromptVariant::default_prompt &&
               state.menu_mode == 1U && state.menu_result == -1 && services == 1 &&
               field_mode_services == 1 && state.caller_exit_status == -1) != 0) {
        return 1;
    }

    state.scene_id = 5U;
    state.submode = 4U;
    state.alternate_result = 1;
    if (expect(select_field_mode_service(state, callbacks) ==
                   FieldModeServiceAction::mode_exit_services_completed &&
               observed.profile ==
                   FieldModeServiceProfile::scene_five_submode_four &&
               observed.prompt_variant ==
                   FieldModeServicePromptVariant::positive_result_prompt &&
               state.menu_mode == 6U && state.menu_result == 2 &&
               field_mode_services == 1 && state.caller_exit_status == -1) != 0) {
        return 2;
    }

    state.scene_id = 1U;
    state.submode = 5U;
    state.alternate_result = 0;
    callback_service_result = 1;
    if (expect(select_field_mode_service(state, callbacks) ==
                   FieldModeServiceAction::field_mode_service_dispatched &&
               state.menu_mode == 2U && state.menu_result == 2 &&
               field_mode_services == 2 && state.caller_exit_status == -1) != 0) {
        return 3;
    }
    callback_service_result = -1;
    if (expect(select_field_mode_service(state, callbacks) ==
                   FieldModeServiceAction::field_mode_service_dispatched &&
               state.menu_result == -1 && field_mode_services == 3 &&
               state.caller_exit_status == -1) != 0) {
        return 4;
    }
    callback_service_result = 0;
    if (expect(select_field_mode_service(state, callbacks) ==
                   FieldModeServiceAction::mode_exit_services_completed &&
               field_mode_services == 3 && state.caller_exit_status == -1) != 0) {
        return 5;
    }

    state.scene_id = 4U;
    state.submode = 3U;
    state.alternate_result = 0;
    callback_service_result = 1;
    if (expect(select_field_mode_service(state, callbacks) ==
                   FieldModeServiceAction::field_mode_service_dispatched &&
               state.menu_mode == 3U && state.menu_result == 0 &&
               field_mode_services == 4 && state.caller_exit_status == -1) != 0) {
        return 6;
    }

    state.scene_id = 9U;
    if (expect(select_field_mode_service(state, callbacks) ==
                   FieldModeServiceAction::no_matching_route && services == 6) != 0) {
        return 7;
    }

    return 0;
}
