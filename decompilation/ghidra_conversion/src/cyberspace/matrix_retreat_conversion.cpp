#include "shadowrun_ghidra/cyberspace/matrix_retreat_conversion.hpp"

namespace shadowrun::ghidra_conversion {

MatrixRetreatRoute handle_matrix_retreat_input(
    MatrixRetreatState& state, const std::uint16_t input_events) {
    if (state.previous_node < 0) {
        return MatrixRetreatRoute::retreat_unavailable;
    }

    constexpr auto action_mask = static_cast<std::uint16_t>(
        (1U << 12) | (1U << 13) | (1U << 14));
    if ((input_events & action_mask) != 0U) {
        const auto next_current = state.previous_node;
        state.previous_node = state.current_node;
        state.current_node = next_current;
        state.transition_requested = true;
        return MatrixRetreatRoute::retreat_to_previous_node;
    }
    if ((input_events & (1U << 15)) != 0U) {
        return MatrixRetreatRoute::return_to_matrix;
    }
    return MatrixRetreatRoute::no_action;
}

} // namespace shadowrun::ghidra_conversion
