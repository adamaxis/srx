#include "shadowrun_ghidra/matrix_utility_conversion.hpp"

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    MatrixUtilityMenuState state{};
    update_matrix_utility_selection(state, -1);
    if (!expect(state.selection == 1U && state.redraw_requested)) {
        return 1;
    }
    state.redraw_requested = false;
    update_matrix_utility_selection(state, 1);
    if (!expect(state.selection == 0U && state.redraw_requested)) {
        return 2;
    }

    if (!expect(choose_matrix_utility_action(0U, false) ==
                    MatrixUtilityActionRoute::show_map &&
                choose_matrix_utility_action(1U, false) ==
                    MatrixUtilityActionRoute::retreat &&
                choose_matrix_utility_action(1U, true) ==
                    MatrixUtilityActionRoute::retreat_blocked_in_combat &&
                choose_matrix_utility_action(2U, false) ==
                    MatrixUtilityActionRoute::no_action)) {
        return 3;
    }
    return 0;
}
