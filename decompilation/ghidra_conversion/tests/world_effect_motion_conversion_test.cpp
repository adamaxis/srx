#include "shadowrun_ghidra/world_effect_motion_conversion.hpp"

#include <optional>

int main() {
    using namespace shadowrun::ghidra_conversion;

    WorldEffectMotionState state{};
    state.frame_countdown = 1U;
    state.motion_phase = 2U;
    state.position_x = 0x1000U;
    state.position_y = 0x2000U;
    state.horizontal_offset = -0x10;
    state.completion_marker = 1U;
    state.transition_state = 0x1234U;
    int resolved = 0;
    int built = 0;
    WorldEffectMotionCallbacks callbacks{};
    callbacks.resolve_sprite_data =
        [&](const std::uint16_t phase, const bool reverse)
        -> std::optional<std::uint32_t> {
        if (phase != 3U || !reverse) {
            resolved += 100;
        }
        ++resolved;
        return 0x4321U;
    };
    callbacks.build_sprite =
        [&](const std::uint32_t token, const std::uint16_t x,
            const std::uint16_t y, const std::uint16_t flags) {
        if (token != 0x4321U || x != 0x0FFU || y != 0x1FEU ||
            flags != 0x2000U) {
            built += 100;
        }
        ++built;
    };
    if (advance_world_effect_motion(state, callbacks) !=
            WorldEffectMotionAction::rendered ||
        state.motion_phase != 3U || state.position_x != 0x0FF0U ||
        state.position_y != 0x1FE6U || resolved != 1 || built != 1) {
        return 1;
    }

    WorldEffectMotionState finish{};
    finish.frame_countdown = 0U;
    finish.completion_marker = 1U;
    finish.transition_state = 2U;
    int serviced = 0;
    callbacks = {};
    callbacks.run_menu_selection_service = [&] { ++serviced; };
    if (advance_world_effect_motion(finish, callbacks) !=
            WorldEffectMotionAction::finalized ||
        finish.completion_marker != 0U || finish.transition_state != 0U ||
        serviced != 1) {
        return 2;
    }
    return 0;
}
