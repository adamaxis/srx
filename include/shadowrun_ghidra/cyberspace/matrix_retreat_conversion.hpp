#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct MatrixRetreatState {
    // FFF118 is the current node and FFF11A is the source previous-node
    // field. A negative previous node is the JACK OUT fallback sentinel.
    std::int16_t current_node = -1;
    std::int16_t previous_node = -1;
    bool transition_requested = false;
};

enum class MatrixRetreatRoute {
    no_action,
    retreat_unavailable,
    retreat_to_previous_node,
    return_to_matrix,
};

// ROM 0x19180..0x19232: process the retreat action/back input against the
// current and previous node fields. Action bits swap the two nodes and set
// the source transition marker; bit 15 returns to the Matrix panel.
MatrixRetreatRoute handle_matrix_retreat_input(
    MatrixRetreatState& state, std::uint16_t input_events);

} // namespace shadowrun::ghidra_conversion
