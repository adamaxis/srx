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
    int refresh_calls = 0;
    const FieldExitPromptCallbacks callbacks{
        [&](FieldExitPromptSelection& selection, FieldExitPromptState&) {
            observed = selection;
            ++service_calls;
        },
        [&] { ++refresh_calls; }};

    if (expect(select_field_exit_prompt(0U, 3U, 0, state, callbacks) ==
                   FieldExitPromptAction::prompt_presented &&
               observed.resource_profile ==
                   FieldExitResourceProfile::scene_zero_submode_three &&
               observed.prompt_asset ==
                   FieldExitPromptAsset::scene_zero_submode_three_default &&
               state.exit_marker == 0x4BU && service_calls == 1 &&
               refresh_calls == 0) != 0) {
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

    if (expect(select_field_exit_prompt(4U, 0U, -1, state, callbacks) ==
                   FieldExitPromptAction::prompt_presented &&
               observed.resource_profile ==
                   FieldExitResourceProfile::scene_four_submode_zero_negative_variant &&
               observed.prompt_asset ==
                   FieldExitPromptAsset::scene_four_submode_zero_negative) != 0) {
        return 4;
    }

    const auto refresh_action = select_field_exit_prompt(
        2U, 2U, 1, state,
        { [&](FieldExitPromptSelection&, FieldExitPromptState& callback_state) {
              callback_state.exit_marker = 0x80U;
          },
          [&] { ++refresh_calls; } });
    if (expect(refresh_action ==
                   FieldExitPromptAction::prompt_presented_and_display_refreshed &&
               refresh_calls == 1) != 0) {
        return 5;
    }

    if (expect(select_field_exit_prompt(9U, 9U, 0, state, callbacks) ==
                   FieldExitPromptAction::no_matching_scene &&
               service_calls == 4) != 0) {
        return 6;
    }

    return 0;
}
