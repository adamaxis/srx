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

std::uint8_t move_wrapped_menu_cursor(
    const std::uint8_t current,
    const std::int8_t step,
    const std::uint8_t maximum) {
    const auto candidate = static_cast<std::uint8_t>(
        static_cast<std::uint16_t>(current) +
        static_cast<std::int16_t>(step));

    // C376 uses ADD.B followed by BGE. A negative signed result means that a
    // decrement crossed the lower edge, so the source loads D3 as the upper
    // cursor value.
    if (static_cast<std::int8_t>(candidate) < 0) {
        return maximum;
    }

    // CMP.B (cursor),D3 followed by BPL is equivalent to testing the signed
    // byte result of maximum - candidate. Values above the upper edge wrap to
    // zero; retaining byte arithmetic also preserves the source boundary for
    // callers that use values near the signed-byte limit.
    const auto distance = static_cast<std::uint8_t>(
        static_cast<std::uint16_t>(maximum) - candidate);
    if (static_cast<std::int8_t>(distance) < 0) {
        return 0;
    }
    return candidate;
}

ActionSelectorUpdate cycle_inventory_action_selector(
    const std::uint8_t current,
    const std::int8_t step) {
    const auto candidate = static_cast<std::int16_t>(current) + step;
    if (candidate < 2) {
        return ActionSelectorUpdate{5, true};
    }
    if (candidate == 6) {
        return ActionSelectorUpdate{2, false};
    }
    return ActionSelectorUpdate{
        static_cast<std::uint8_t>(candidate), false};
}

} // namespace shadowrun::ghidra_conversion
