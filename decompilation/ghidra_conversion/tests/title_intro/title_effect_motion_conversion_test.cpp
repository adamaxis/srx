#include "shadowrun_ghidra/title_intro/title_effect_motion_conversion.hpp"

#include <optional>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleEffectMotionState state{};
    state.frame_delay = 0U;
    state.position_x = 0x100U;
    state.position_y = 0x200U;
    state.effect_mode = 1U;
    int resolves = 0;
    int builds = 0;
    TitleEffectMotionCallbacks callbacks{};
    callbacks.resolve_sprite_data =
        [&](const std::uint16_t mode, const std::uint16_t frame,
            const std::uint16_t x) -> std::optional<std::uint32_t> {
        if (mode != 1U || frame != 1U || x != 0x100U) {
            resolves += 100;
        }
        ++resolves;
        return 0x4321U;
    };
    callbacks.build_sprite =
        [&](const std::uint32_t token, const std::uint16_t x,
            const std::uint16_t y, const std::uint16_t flags) {
        if (token != 0x4321U || x != 0x100U || y != 0x200U ||
            flags != 0x2000U) {
            builds += 100;
        }
        ++builds;
    };
    if (advance_title_effect_motion(state, callbacks) !=
            TitleEffectMotionAction::rendered ||
        state.frame_delay != 3U || state.frame != 1U || resolves != 1 ||
        builds != 1) {
        return 1;
    }

    TitleEffectMotionState finish{};
    finish.frame_delay = 0U;
    finish.frame = 0x0BU;
    finish.position_x = 0x011FU;
    finish.transition_state = 0x1234U;
    finish.menu_state = 0x5678U;
    int published = 0;
    int serviced = 0;
    callbacks = {};
    callbacks.publish_source_record =
        [&](const std::uint32_t token, const std::uint8_t phase) {
        published += token == 0x1C288U && phase == 0U ? 1 : 100;
    };
    callbacks.run_menu_selection_service = [&] { ++serviced; };
    if (advance_title_effect_motion(finish, callbacks) !=
            TitleEffectMotionAction::finalized ||
        finish.first_source_token != 0x1C288U || finish.first_phase != 0U ||
        finish.transition_state != 0U || finish.menu_state != 0U ||
        published != 1 || serviced != 1) {
        return 2;
    }
    return 0;
}
