#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

enum class FieldExitResourceProfile {
    scene_zero_submode_three,
    scene_one_submode_one,
    scene_two_submode_two,
    scene_three_submode_one,
    scene_four_submode_zero,
    scene_four_submode_zero_negative_variant,
};

enum class FieldExitPromptAsset {
    scene_zero_submode_three_default,
    scene_zero_submode_three_positive,
    scene_zero_submode_three_negative,
    scene_one_submode_one_default,
    scene_one_submode_one_positive,
    scene_one_submode_one_negative,
    scene_one_submode_one_flagged,
    scene_two_submode_two_default,
    scene_two_submode_two_positive,
    scene_two_submode_two_negative,
    scene_three_submode_one_default,
    scene_three_submode_one_positive,
    scene_three_submode_one_negative,
    scene_four_submode_zero_default,
    scene_four_submode_zero_positive,
    scene_four_submode_zero_negative,
};

struct FieldExitPromptState {
    std::uint8_t exit_marker = 0;
    std::uint32_t world_flags = 0;
};

struct FieldExitPromptSelection {
    FieldExitResourceProfile resource_profile =
        FieldExitResourceProfile::scene_zero_submode_three;
    FieldExitPromptAsset prompt_asset =
        FieldExitPromptAsset::scene_zero_submode_three_default;
    std::uint8_t exit_marker = 0;
};

struct FieldExitPromptCallbacks {
    // The selected profile and prompt asset are already native identifiers;
    // the coordinator binds them to the host resource/alias system.
    std::function<void(FieldExitPromptSelection&, FieldExitPromptState&)>
        run_mode_exit_services;
    std::function<void()> refresh_mode_display;
};

enum class FieldExitPromptAction {
    no_matching_scene,
    prompt_presented,
    prompt_presented_and_display_refreshed,
};

// ROM 0x1F65A: select the field-exit prompt/resource pair for the current
// scene/submode and alternate-result marker, then run the exit services. The
// source's FFFC1E bits 13 and 30 are modeled as ordinary world flags.
FieldExitPromptAction select_field_exit_prompt(
    std::uint8_t scene_id,
    std::uint8_t submode,
    std::int8_t alternate_result,
    FieldExitPromptState& state,
    const FieldExitPromptCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
