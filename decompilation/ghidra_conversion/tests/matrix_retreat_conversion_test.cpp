#include "shadowrun_ghidra/matrix_retreat_conversion.hpp"

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    MatrixRetreatState unavailable{12, -1, false};
    if (!expect(handle_matrix_retreat_input(unavailable, 1U << 14) ==
                    MatrixRetreatRoute::retreat_unavailable &&
                unavailable.current_node == 12 &&
                unavailable.previous_node == -1 &&
                !unavailable.transition_requested)) {
        return 1;
    }

    MatrixRetreatState state{12, 8, false};
    if (!expect(handle_matrix_retreat_input(state, 1U << 13) ==
                    MatrixRetreatRoute::retreat_to_previous_node &&
                state.current_node == 8 && state.previous_node == 12 &&
                state.transition_requested)) {
        return 2;
    }
    state.transition_requested = false;
    if (!expect(handle_matrix_retreat_input(state, 1U << 15) ==
                    MatrixRetreatRoute::return_to_matrix &&
                !state.transition_requested)) {
        return 3;
    }
    if (!expect(handle_matrix_retreat_input(state, 0U) ==
                MatrixRetreatRoute::no_action)) {
        return 4;
    }
    return 0;
}
