#include "shadowrun_ghidra/field/field_exit_prompt_conversion.hpp"

#include <cstdint>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldExitPromptState state{};
    FieldExitPromptSelection observed{};
    int service_calls = 0;
    int hotel_menu_calls = 0;
    const FieldExitPromptCallbacks callbacks{
        [&](FieldExitPromptSelection& selection, FieldExitPromptState&) {
            observed = selection;
            ++service_calls;
        },
        [&] { ++hotel_menu_calls; }};

    if (expect(select_field_exit_prompt(0U, 3U, 0, state, callbacks) ==
                   FieldExitPromptAction::prompt_presented &&
               observed.resource_profile ==
                   FieldExitResourceProfile::scene_zero_submode_three &&
               observed.prompt_asset ==
                   FieldExitPromptAsset::scene_zero_submode_three_default &&
               state.exit_marker == 0x4BU && service_calls == 1 &&
               hotel_menu_calls == 0 && state.caller_exit_status == -1) != 0) {
        return 1;
    }

    state.world_flags = (1UL << 13U);
    if (expect(select_field_exit_prompt(1U, 1U, 0, state, callbacks) ==
                   FieldExitPromptAction::prompt_presented &&
               observed.prompt_asset ==
                   FieldExitPromptAsset::scene_one_submode_one_flagged) != 0) {
        return 2;
    }
    state.world_flags |= (1UL << 30U);
    if (expect(select_field_exit_prompt(1U, 1U, 0, state, callbacks) ==
                   FieldExitPromptAction::prompt_presented &&
               observed.prompt_asset ==
                   FieldExitPromptAsset::scene_one_submode_one_default) != 0) {
        return 3;
    }

    if (expect(select_field_exit_prompt(2U, 2U, 0, state, callbacks) ==
                   FieldExitPromptAction::prompt_presented &&
               observed.resource_profile ==
                   FieldExitResourceProfile::stoker_motel &&
               observed.prompt_asset ==
                   FieldExitPromptAsset::stoker_motel_arrival &&
               observed.exit_marker == 0x14U) != 0) {
        return 8;
    }
    if (expect(select_field_exit_prompt(2U, 2U, 1, state, callbacks) ==
                   FieldExitPromptAction::prompt_presented &&
               observed.prompt_asset ==
                   FieldExitPromptAsset::stoker_motel_package_question) != 0) {
        return 9;
    }
    if (expect(select_field_exit_prompt(2U, 2U, -1, state, callbacks) ==
                   FieldExitPromptAction::prompt_presented &&
               observed.prompt_asset ==
                   FieldExitPromptAsset::stoker_motel_package_handoff) != 0) {
        return 10;
    }

    if (expect(select_field_exit_prompt(4U, 0U, -1, state, callbacks) ==
                   FieldExitPromptAction::prompt_presented &&
               observed.resource_profile ==
                   FieldExitResourceProfile::scene_four_submode_zero_negative_variant &&
               observed.prompt_asset ==
                   FieldExitPromptAsset::scene_four_submode_zero_negative) != 0) {
        return 4;
    }

    state.caller_exit_status = 0;
    const auto hotel_action = select_field_exit_prompt(
        2U, 2U, 1, state,
        { [&](FieldExitPromptSelection&, FieldExitPromptState& callback_state) {
              callback_state.exit_marker = 0x80U;
          },
          [&] { ++hotel_menu_calls; } });
    if (expect(hotel_action == FieldExitPromptAction::hotel_options_menu_entered &&
               hotel_menu_calls == 1 && state.caller_exit_status == -1) != 0) {
        return 5;
    }

    state.caller_exit_status = 0;
    const auto unresolved_hotel_action = select_field_exit_prompt(
        2U, 2U, 1, state,
        { [&](FieldExitPromptSelection&, FieldExitPromptState& callback_state) {
              callback_state.exit_marker = 0x80U;
          },
          {} });
    if (expect(unresolved_hotel_action ==
                   FieldExitPromptAction::hotel_options_menu_unresolved &&
               state.caller_exit_status == 0) != 0) {
        return 6;
    }

    if (expect(select_field_exit_prompt(9U, 9U, 0, state, callbacks) ==
                   FieldExitPromptAction::no_matching_scene &&
               service_calls == 7) != 0) {
        return 7;
    }

    return 0;
}
