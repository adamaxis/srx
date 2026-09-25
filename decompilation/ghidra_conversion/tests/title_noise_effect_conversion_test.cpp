#include "shadowrun_ghidra/title_noise_effect_conversion.hpp"

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
    std::vector<TitleNoiseEffectSpriteBuild> builds;
    callbacks = {};
    callbacks.resolve_sprite_data =
        [](const std::uint32_t table) -> std::optional<std::uint32_t> {
        return table + 8U;
    };
    callbacks.build_sprite = [&](const TitleNoiseEffectSpriteBuild& build) {
        builds.push_back(build);
    };
    if (advance_title_noise_effect(active, callbacks) !=
            TitleNoiseEffectAction::rendered ||
        active.countdown != 7U || builds.size() != 2U ||
        builds[0].source_table_token != 0x1DAE0U ||
        builds[0].x != 0x158U || builds[1].source_table_token != 0x1DAEAU ||
        builds[1].x != 0xE7U) {
        return 2;
    }

    TitleNoiseEffectState random_state{};
    random_state.countdown = 1U;
    callbacks = {};
    callbacks.advance_source_prng =
        [] { return std::optional<std::uint16_t>(0x0001U); };
    callbacks.resolve_sprite_data =
        [](const std::uint32_t table) -> std::optional<std::uint32_t> {
        return table + 8U;
    };
    callbacks.build_sprite = [&](const TitleNoiseEffectSpriteBuild& build) {
        builds.push_back(build);
    };
    builds.clear();
    if (advance_title_noise_effect(random_state, callbacks) !=
            TitleNoiseEffectAction::rendered ||
        builds.size() != 7U || builds[0].x != 0xE8U ||
        builds[0].y != 0xECU || random_state.random_word != 0x0004U) {
        return 3;
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
        return 4;
    }
    return 0;
}
