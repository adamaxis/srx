#include "shadowrun_ghidra/field_exit_prompt_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

FieldExitPromptSelection select_scene_zero_submode_three(
    const std::int8_t alternate_result) {
    if (alternate_result > 0) {
        return {FieldExitResourceProfile::scene_zero_submode_three,
                FieldExitPromptAsset::scene_zero_submode_three_positive, 0x4BU};
    }
    if (alternate_result < 0) {
        return {FieldExitResourceProfile::scene_zero_submode_three,
                FieldExitPromptAsset::scene_zero_submode_three_negative, 0x4BU};
    }
    return {FieldExitResourceProfile::scene_zero_submode_three,
            FieldExitPromptAsset::scene_zero_submode_three_default, 0x4BU};
}

FieldExitPromptSelection select_scene_one_submode_one(
    const std::int8_t alternate_result, const std::uint32_t world_flags) {
    if (alternate_result > 0) {
        return {FieldExitResourceProfile::scene_one_submode_one,
                FieldExitPromptAsset::scene_one_submode_one_positive, 0x1EU};
    }
    if (alternate_result < 0) {
        return {FieldExitResourceProfile::scene_one_submode_one,
                FieldExitPromptAsset::scene_one_submode_one_negative, 0x1EU};
    }
    if ((world_flags & (1UL << 13U)) != 0U &&
        (world_flags & (1UL << 30U)) == 0U) {
        return {FieldExitResourceProfile::scene_one_submode_one,
                FieldExitPromptAsset::scene_one_submode_one_flagged, 0x1EU};
    }
    return {FieldExitResourceProfile::scene_one_submode_one,
            FieldExitPromptAsset::scene_one_submode_one_default, 0x1EU};
}

} // namespace

FieldExitPromptAction select_field_exit_prompt(
    const std::uint8_t scene_id,
    const std::uint8_t submode,
    const std::int8_t alternate_result,
    FieldExitPromptState& state,
    const FieldExitPromptCallbacks& callbacks) {
    FieldExitPromptSelection selection{};
    bool matched = true;

    if (scene_id == 0U && submode == 3U) {
        selection = select_scene_zero_submode_three(alternate_result);
    } else if (scene_id == 1U && submode == 1U) {
        selection = select_scene_one_submode_one(alternate_result,
                                                 state.world_flags);
    } else if (scene_id == 2U && submode == 2U) {
        selection.resource_profile =
            FieldExitResourceProfile::scene_two_submode_two;
        selection.exit_marker = 0x14U;
        if (alternate_result > 0) {
            selection.prompt_asset =
                FieldExitPromptAsset::scene_two_submode_two_positive;
        } else if (alternate_result < 0) {
            selection.prompt_asset =
                FieldExitPromptAsset::scene_two_submode_two_negative;
        } else {
            selection.prompt_asset =
                FieldExitPromptAsset::scene_two_submode_two_default;
        }
    } else if (scene_id == 3U && submode == 1U) {
        selection.resource_profile =
            FieldExitResourceProfile::scene_three_submode_one;
        selection.exit_marker = 0x32U;
        if (alternate_result > 0) {
            selection.prompt_asset =
                FieldExitPromptAsset::scene_three_submode_one_positive;
        } else if (alternate_result < 0) {
            selection.prompt_asset =
                FieldExitPromptAsset::scene_three_submode_one_negative;
        } else {
            selection.prompt_asset =
                FieldExitPromptAsset::scene_three_submode_one_default;
        }
    } else if (scene_id == 4U && submode == 0U) {
        selection.resource_profile =
            FieldExitResourceProfile::scene_four_submode_zero;
        selection.exit_marker = 0x28U;
        if (alternate_result > 0) {
            selection.prompt_asset =
                FieldExitPromptAsset::scene_four_submode_zero_positive;
        } else if (alternate_result < 0) {
            selection.resource_profile =
                FieldExitResourceProfile::scene_four_submode_zero_negative_variant;
            selection.prompt_asset =
                FieldExitPromptAsset::scene_four_submode_zero_negative;
        } else {
            selection.prompt_asset =
                FieldExitPromptAsset::scene_four_submode_zero_default;
        }
    } else {
        matched = false;
    }

    if (!matched) {
        return FieldExitPromptAction::no_matching_scene;
    }

    state.exit_marker = selection.exit_marker;
    if (callbacks.run_mode_exit_services) {
        callbacks.run_mode_exit_services(selection, state);
    }
    if (static_cast<std::int8_t>(state.exit_marker) < 0) {
        if (callbacks.refresh_mode_display) {
            callbacks.refresh_mode_display();
        }
        return FieldExitPromptAction::prompt_presented_and_display_refreshed;
    }
    return FieldExitPromptAction::prompt_presented;
}

} // namespace shadowrun::ghidra_conversion
