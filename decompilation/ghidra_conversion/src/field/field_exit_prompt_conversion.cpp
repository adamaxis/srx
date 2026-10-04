#include "shadowrun_ghidra/field/field_exit_prompt_conversion.hpp"

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
    const std::uint8_t field_mode,
    const std::uint8_t event_code,
    const std::int8_t alternate_result,
    FieldExitPromptState& state,
    const FieldExitPromptCallbacks& callbacks) {
    FieldExitPromptSelection selection{};
    bool matched = true;

    if (field_mode == 0U && event_code == 3U) {
        selection = select_scene_zero_submode_three(alternate_result);
    } else if (field_mode == 1U && event_code == 1U) {
        selection = select_scene_one_submode_one(alternate_result,
                                                 state.world_flags);
    } else if (field_mode == 2U && event_code == 2U) {
        selection.resource_profile =
            FieldExitResourceProfile::stoker_motel;
        selection.exit_marker = 0x14U;
        if (alternate_result > 0) {
            selection.prompt_asset =
                FieldExitPromptAsset::stoker_motel_package_question;
        } else if (alternate_result < 0) {
            selection.prompt_asset =
                FieldExitPromptAsset::stoker_motel_package_handoff;
        } else {
            selection.prompt_asset =
                FieldExitPromptAsset::stoker_motel_arrival;
        }
    } else if (field_mode == 3U && event_code == 1U) {
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
    } else if (field_mode == 4U && event_code == 0U) {
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
        if (!callbacks.run_hotel_options_menu) {
            return FieldExitPromptAction::hotel_options_menu_unresolved;
        }
        callbacks.run_hotel_options_menu();
        state.caller_exit_status = -1;
        return FieldExitPromptAction::hotel_options_menu_entered;
    }
    state.caller_exit_status = -1;
    return FieldExitPromptAction::prompt_presented;
}

} // namespace shadowrun::ghidra_conversion
