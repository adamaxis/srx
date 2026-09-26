#include "shadowrun_ghidra/cyberspace/matrix_utility_conversion.hpp"

#include "shadowrun_ghidra/core/selection_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void update_matrix_utility_selection(MatrixUtilityMenuState& state,
                                     const std::int8_t step) {
    const auto next = move_wrapped_menu_cursor(state.selection, step, 1U);
    if (next == state.selection) {
        return;
    }
    state.selection = next;
    state.redraw_requested = true;
}

MatrixUtilityActionRoute choose_matrix_utility_action(
    const std::uint8_t selection, const bool in_combat) {
    if (selection == 0U) {
        return MatrixUtilityActionRoute::show_map;
    }
    if (selection == 1U) {
        return in_combat ? MatrixUtilityActionRoute::retreat_blocked_in_combat
                         : MatrixUtilityActionRoute::retreat;
    }
    return MatrixUtilityActionRoute::no_action;
}

} // namespace shadowrun::ghidra_conversion
