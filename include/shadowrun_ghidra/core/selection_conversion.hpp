#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct SelectionState {
    std::int8_t column = 0;
    std::int8_t row = 0;
};

struct ActionSelectorUpdate {
    std::uint8_t selector = 2;
    bool redraw_requested = false;
};

// ROM 0xFEBE: move the active column by a signed step, wrapping between zero
// and the current row's maximum column.
void move_selection_column(
    SelectionState& state,
    std::int8_t step,
    std::uint8_t available_entries);

// ROM 0xFF06: move the two-row selection mode when at least seven entries
// exist, then clear the row when the active column is unavailable there.
void move_selection_row(
    SelectionState& state,
    std::int8_t step,
    std::uint8_t available_entries);

// ROM 0xC376: update the shared menu cursor by a signed byte step. The
// source treats both the candidate and the upper bound as signed bytes when
// deciding whether to wrap, so the host helper preserves the byte arithmetic
// while exposing only the state mutation. The paired VDP writes at 0xC3B2
// remain an explicit presentation boundary.
std::uint8_t move_wrapped_menu_cursor(
    std::uint8_t current,
    std::int8_t step,
    std::uint8_t maximum);

// ROM 0x118C6: cycle the inventory/clips action selector in the inclusive
// range 2..5. The source marks a redraw only when a leftward step wraps below
// the first action.
ActionSelectorUpdate cycle_inventory_action_selector(
    std::uint8_t current,
    std::int8_t step);

} // namespace shadowrun::ghidra_conversion
