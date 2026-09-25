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

struct FieldModeServiceState {
    std::uint8_t scene_id = 0;
    std::uint8_t submode = 0;
    std::uint8_t menu_mode = 0; // FFF0B6
    std::int8_t menu_result = 0; // FFF0B7
    std::int8_t alternate_result = 0; // FFF0D5
    std::int8_t service_result = 0; // FFF112
};

struct FieldModeServiceSelection {
    FieldModeServiceProfile profile =
        FieldModeServiceProfile::scene_two_submode_eleven;
    FieldModeServicePromptVariant prompt_variant =
        FieldModeServicePromptVariant::default_prompt;
    std::uint8_t menu_mode = 0;
    std::int8_t menu_result = 0;
};

struct FieldModeServiceCallbacks {
    std::function<void(FieldModeServiceSelection&, FieldModeServiceState&)>
        run_mode_exit_services;
    std::function<void(FieldModeServiceState&)> run_intro_stage;
};

enum class FieldModeServiceAction {
    no_matching_route,
    service_only,
    intro_stage_started,
};

// ROM 0x1FA78: select a mode-service profile and prompt variant, preserve
// menu result fields, and route the source's service-result handshake before
// optionally entering the intro-stage boundary.
FieldModeServiceAction select_field_mode_service(
    FieldModeServiceState& state,
    const FieldModeServiceCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
