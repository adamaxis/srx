#include "shadowrun_ghidra/selection_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void move_selection_column(
    SelectionState& state,
    const std::int8_t step,
    const std::uint8_t available_entries) {
    auto maximum_column = static_cast<std::int16_t>(available_entries);
    if (state.row == 0 && maximum_column >= 7) {
        maximum_column = 6;
    } else if (state.row != 0) {
        maximum_column -= 7;
    }

    auto next_column = static_cast<std::int16_t>(state.column) + step;
    if (next_column < 0) {
        next_column = maximum_column;
    } else if (next_column > maximum_column) {
        next_column = 0;
    }
    state.column = static_cast<std::int8_t>(next_column);
}

void move_selection_row(
    SelectionState& state,
    const std::int8_t step,
    const std::uint8_t available_entries) {
    if (available_entries < 7) {
        return;
    }

    auto next_row = static_cast<std::int16_t>(state.row) + step;
    if (next_row < 0) {
        next_row = 1;
    } else if (next_row == 2) {
        next_row = 0;
    } else if (next_row == 0) {
        state.row = 0;
        return;
    }

    if (static_cast<std::int16_t>(available_entries) - 7 -
            static_cast<std::int16_t>(state.column) <
        0) {
        next_row = 0;
    }
    state.row = static_cast<std::int8_t>(next_row);
}

} // namespace shadowrun::ghidra_conversion
