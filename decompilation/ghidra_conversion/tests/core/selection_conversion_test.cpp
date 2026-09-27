#include "shadowrun_ghidra/core/selection_conversion.hpp"

#include <cstdint>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    SelectionState columns{0, 0};
    move_selection_column(columns, 1, 10);
    if (!expect(columns.column == 1)) {
        return 1;
    }
    move_selection_column(columns, -2, 10);
    if (!expect(columns.column == 6)) {
        return 2;
    }
    move_selection_column(columns, 1, 10);
    if (!expect(columns.column == 0)) {
        return 3;
    }

    columns = SelectionState{2, 1};
    move_selection_column(columns, 1, 10);
    if (!expect(columns.column == 3)) {
        return 4;
    }
    move_selection_column(columns, 1, 10);
    if (!expect(columns.column == 0)) {
        return 5;
    }

    SelectionState rows{0, 0};
    move_selection_row(rows, 1, 10);
    if (!expect(rows.row == 1)) {
        return 6;
    }
    move_selection_row(rows, 1, 10);
    if (!expect(rows.row == 0)) {
        return 7;
    }
    rows = SelectionState{4, 0};
    move_selection_row(rows, 1, 10);
    if (!expect(rows.row == 0)) {
        return 8;
    }
    rows = SelectionState{0, 0};
    move_selection_row(rows, 1, 6);
    if (!expect(rows.row == 0)) {
        return 9;
    }

    if (!expect(move_wrapped_menu_cursor(0, -1, 2) == 2)) {
        return 10;
    }
    if (!expect(move_wrapped_menu_cursor(1, 1, 2) == 2)) {
        return 11;
    }
    if (!expect(move_wrapped_menu_cursor(2, 1, 2) == 0)) {
        return 12;
    }
    if (!expect(move_wrapped_menu_cursor(1, 0, 2) == 1)) {
        return 13;
    }
    // The source BGE uses ADD.B's overflow-corrected signed result: positive
    // overflow does not select the lower wrap, while negative overflow does.
    if (!expect(move_wrapped_menu_cursor(0x7FU, 1, 2) == 0)) {
        return 14;
    }
    if (!expect(move_wrapped_menu_cursor(0x80U, -1, 2) == 2)) {
        return 15;
    }

    const auto action_left = cycle_inventory_action_selector(2, -1);
    if (!expect(action_left.selector == 5 &&
                action_left.redraw_requested)) {
        return 16;
    }
    const auto action_right = cycle_inventory_action_selector(5, 1);
    if (!expect(action_right.selector == 2 &&
                !action_right.redraw_requested)) {
        return 17;
    }
    const auto action_middle = cycle_inventory_action_selector(3, -1);
    if (!expect(action_middle.selector == 2 &&
                !action_middle.redraw_requested)) {
        return 18;
    }

    return 0;
}
