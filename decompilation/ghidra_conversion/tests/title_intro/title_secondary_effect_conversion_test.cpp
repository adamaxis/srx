#include "shadowrun_ghidra/title_intro/title_secondary_effect_conversion.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

using namespace shadowrun::ghidra_conversion;

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    if (resolve_title_secondary_effect_source_table(0U, 0U) != 0x1D906U ||
        resolve_title_secondary_effect_source_table(1U, 2U) != 0x1D948U ||
        resolve_title_secondary_effect_source_table(2U, 1U) != 0x1D95CU ||
        resolve_title_secondary_effect_source_table(7U, 0U) != 0x1D952U ||
        resolve_title_secondary_effect_source_table(0U, 3U).has_value()) {
        return 1;
    }

    TitleSecondaryEffectState state{};
    state.active = true;
    state.phase_delay = 2U;
    state.animation_phase = 0U;
    state.source_variant = 0U;
    state.frame_delay = 0x15U;
    state.lifetime = 0x3CU;
    state.position_x = 0x0140;
    state.position_y = 0x1020;

    std::vector<TitleSecondaryEffectSpriteBuild> builds;
    std::uint32_t progressed = 0;
    TitleSecondaryEffectCallbacks callbacks{};
    callbacks.resolve_sprite_asset = [](const std::uint32_t source) {
        return std::optional<std::uint32_t>(source + 0x100000U);
    };
    callbacks.build_sprite =
        [&](const TitleSecondaryEffectSpriteBuild& build) {
        builds.push_back(build);
    };
    callbacks.run_menu_progression_variant = [&] { ++progressed; };

    if (!expect(advance_title_secondary_effect_service(state, callbacks) ==
                    TitleSecondaryEffectAction::rendered &&
                builds.size() == 1U && builds[0].source_table_token ==
                    0x1D906U && builds[0].sprite_asset_token == 0x11D906U &&
                builds[0].x == 0x13U && builds[0].y == 0x101U &&
                state.animation_phase == 0U && state.phase_delay == 1U)) {
        return 2;
    }

    state.frame_delay = 1U;
    state.source_variant = 0U;
    state.animation_phase = 0U;
    state.phase_delay = 1U;
    state.paired_positions = {{{0x100, 0x200}, {0x120, 0x220}}};
    builds.clear();
    if (!expect(advance_title_secondary_effect_service(state, callbacks) ==
                    TitleSecondaryEffectAction::rendered &&
                state.source_variant == 0U && state.frame_delay == 0U &&
                state.paired_positions[0].x == state.position_x &&
                state.paired_positions[1].y == state.position_y &&
                builds.size() == 1U)) {
        return 3;
    }

    state.source_variant = 1U;
    state.frame_delay = 0U;
    state.phase_delay = 2U;
    state.animation_phase = 0U;
    state.position_x = 0x0200;
    state.position_y = 0x0400;
    state.paired_positions = {{{0x0600, 0x0800}, {0x0A00, 0x0C00}}};
    builds.clear();
    if (!expect(advance_title_secondary_effect_service(state, callbacks) ==
                    TitleSecondaryEffectAction::rendered &&
                state.source_variant == 2U && builds.size() == 9U)) {
        return 4;
    }

    state.active = true;
    state.lifetime = 0U;
    state.alternate_motion = true;
    state.title_service_flags = 0xFFU;
    state.transition_state = 7U;
    state.frame_delay = 0x14U;
    state.phase_delay = 2U;
    state.source_variant = 0U;
    state.animation_phase = 0U;
    builds.clear();
    if (!expect(advance_title_secondary_effect_service(state, callbacks) ==
                    TitleSecondaryEffectAction::expired && !state.active &&
                state.burst_countdown == 0x0AU &&
                (state.title_service_flags & 0x40U) == 0U &&
                state.transition_state == 0U && progressed == 1U)) {
        return 5;
    }

    state.active = true;
    state.lifetime = 0x3CU;
    callbacks.resolve_sprite_asset =
        [](const std::uint32_t) -> std::optional<std::uint32_t> {
        return std::nullopt;
    };
    if (!expect(advance_title_secondary_effect_service(state, callbacks) ==
                TitleSecondaryEffectAction::sprite_asset_unresolved)) {
        return 6;
    }
    return 0;
}
