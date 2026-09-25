#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct MatrixUtilityMenuState {
    // The source C376 cursor is bounded by one: 0 = Show Map, 1 = Retreat.
    std::uint8_t selection = 0;
    bool redraw_requested = false;
};

// ROM 0x19054..0x1909A: cycle the two Matrix utility entries with the shared
// menu cursor behavior.
void update_matrix_utility_selection(MatrixUtilityMenuState& state,
                                     std::int8_t step);

enum class MatrixUtilityActionRoute {
    no_action,
    show_map,
    retreat,
    retreat_blocked_in_combat,
};

// ROM 0x1909C..0x19110: action bits dispatch selection zero to the map path;
// selection one goes to retreat unless the source combat gate is active.
MatrixUtilityActionRoute choose_matrix_utility_action(
    std::uint8_t selection, bool in_combat);

} // namespace shadowrun::ghidra_conversion
