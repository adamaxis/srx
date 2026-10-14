#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

enum class FieldModeServiceProfile {
    scene_two_submode_eleven,
    scene_one_submode_five,
    scene_one_submode_three,
    scene_five_submode_two,
    scene_five_submode_three,
    scene_five_submode_four,
    scene_five_submode_five,
    scene_four_submode_three,
};

enum class FieldModeServicePromptVariant {
    default_prompt,
    positive_result_prompt,
    negative_result_prompt,
};

struct FieldModeServiceRouteState {
    std::uint8_t scene_id = 0;
    std::uint8_t submode = 0;
    std::uint8_t menu_mode = 0; // FFF0B6
    std::int8_t menu_result = 0; // FFF0B7
    std::int8_t alternate_result = 0; // FFF0D5
    std::int8_t service_result = 0; // FFF112
    std::int16_t caller_exit_status = 0; // D6
};

struct FieldModeServiceSelection {
    FieldModeServiceProfile profile =
        FieldModeServiceProfile::scene_two_submode_eleven;
    FieldModeServicePromptVariant prompt_variant =
        FieldModeServicePromptVariant::default_prompt;
    std::uint8_t menu_mode = 0;
    std::int8_t menu_result = 0;
};

struct FieldModeServiceRouteCallbacks {
    std::function<void(FieldModeServiceSelection&,
                       FieldModeServiceRouteState&)>
        run_mode_exit_services;
    // ROM 0x56AC0: the default route dispatches the field-mode service after
    // 0x1F246 has prepared the selected mode.
    std::function<void(FieldModeServiceRouteState&)> run_field_mode_service;
};

enum class FieldModeServiceAction {
    no_matching_route,
    mode_exit_services_unresolved,
    mode_exit_services_completed,
    field_mode_service_unresolved,
    field_mode_service_dispatched,
};

// ROM 0x1FA78: select a mode-service profile and prompt variant, preserve
// the source menu fields, route the FFF112 service-result handshake, and
// dispatch the named 0x56AC0 field-mode service where the ROM does so. Every
// completed source route publishes D6=-1 to the caller.
FieldModeServiceAction select_field_mode_service(
    FieldModeServiceRouteState& state,
    const FieldModeServiceRouteCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
