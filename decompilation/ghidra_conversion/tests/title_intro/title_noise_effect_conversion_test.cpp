#include "shadowrun_ghidra/title_intro/title_noise_effect_conversion.hpp"

#include <optional>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleNoiseEffectState inactive{};
    inactive.countdown = 0U;
    TitleNoiseEffectCallbacks callbacks{};
    if (advance_title_noise_effect(inactive, callbacks) !=
            TitleNoiseEffectAction::updated ||
        (inactive.behavior_flags & 0x04U) == 0U ||
        inactive.countdown != 0x3CU) {
        return 1;
    }

    TitleNoiseEffectState active{};
    active.behavior_flags = 0x04U;
    active.countdown = 0x0008U;
    active.random_word = 0x0001U;
    std::vector<TitleNoiseEffectSpriteBuild> builds;
    callbacks = {};
    callbacks.build_sprite = [&](const TitleNoiseEffectSpriteBuild& build) {
        builds.push_back(build);
    };
    if (advance_title_noise_effect(active, callbacks) !=
            TitleNoiseEffectAction::rendered ||
        active.countdown != 7U || builds.size() != 9U ||
        builds[0].source_table_token != 0x1DAE0U ||
        builds[0].sprite_data_token != 0x1DAE0U ||
        builds[0].x != 0x158U || builds[1].source_table_token != 0x1DAEAU ||
        builds[1].sprite_data_token != 0x1DAEAU || builds[1].x != 0xE7U ||
        builds[2].source_table_token != 0x1DAC2U ||
        builds[2].sprite_data_token != 0x1DAC2U || builds[2].x != 0xE8U ||
        builds[8].source_table_token != 0x1DAB8U ||
        builds[8].sprite_data_token != 0x1DAB8U ||
        builds[8].x != 0x148U) {
        return 2;
    }

    TitleNoiseEffectState active_row_only{};
    active_row_only.behavior_flags = 0x04U;
    active_row_only.countdown = 0x0004U;
    active_row_only.random_word = 0x0000U;
    builds.clear();
    if (advance_title_noise_effect(active_row_only, callbacks) !=
            TitleNoiseEffectAction::rendered ||
        builds.size() != 7U || builds[0].x != 0xE8U ||
        builds[0].source_table_token != 0x1DAB8U) {
        return 3;
    }

    TitleNoiseEffectState random_state{};
    random_state.countdown = 1U;
    callbacks = {};
    callbacks.advance_source_prng =
        [] { return std::optional<std::uint16_t>(0x0001U); };
    callbacks.build_sprite = [&](const TitleNoiseEffectSpriteBuild& build) {
        builds.push_back(build);
    };
    builds.clear();
    if (advance_title_noise_effect(random_state, callbacks) !=
            TitleNoiseEffectAction::rendered ||
        builds.size() != 7U ||
        builds[0].source_table_token != 0x1DAC2U ||
        builds[0].sprite_data_token != 0x1DAC2U ||
        builds[0].x != 0xE8U || builds[0].y != 0xECU ||
        builds[1].source_table_token != 0x1DAB8U ||
        builds[1].x != 0xF8U || builds[5].source_table_token != 0x1DAB8U ||
        builds[5].x != 0x138U || builds[6].source_table_token != 0x1DAB8U ||
        builds[6].x != 0x148U || random_state.random_word != 0x0001U) {
        return 4;
    }

    TitleNoiseEffectState finish{};
    finish.behavior_flags = 0xFFU;
    finish.countdown = 0U;
    finish.transition_state = 0x1234U;
    int finalized = 0;
    callbacks = {};
    callbacks.attach_behavior_descriptor_e = [&] { ++finalized; };
    callbacks.refresh_menu_selection_after_clear = [&] { ++finalized; };
    if (advance_title_noise_effect(finish, callbacks) !=
        TitleNoiseEffectAction::finalized ||
        finish.behavior_flags != 0xFAU || finish.transition_state != 0U ||
        finish.completion_state != 1U || finalized != 2) {
        return 5;
    }
    return 0;
}
