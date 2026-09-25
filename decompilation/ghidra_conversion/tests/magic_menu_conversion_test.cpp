#include "shadowrun_ghidra/magic_menu_conversion.hpp"

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    RunnerRecordFields shaman{};
    shaman.archetype_index = 1U;
    shaman.auxiliary[0] = 6U;
    shaman.magic_table_index = 3U;
    if (!expect(select_magic_menu_path(shaman) == MagicMenuPath::spell_panel)) {
        return 1;
    }

    shaman.auxiliary[0] = 0U;
    if (!expect(select_magic_menu_path(shaman) ==
                    MagicMenuPath::fallback_menu)) {
        return 2;
    }
    shaman.auxiliary[0] = 6U;
    shaman.archetype_index = 2U;
    if (!expect(select_magic_menu_path(shaman) ==
                    MagicMenuPath::fallback_menu)) {
        return 3;
    }

    MagicMenuSelection selection{6U, 0U};
    selection = move_magic_selection_column(selection, 1, 7U);
    if (!expect(selection.column == 0U && selection.row == 0U)) {
        return 4;
    }
    selection = move_magic_selection_row(selection, 1, 7U);
    if (!expect(selection.column == 0U && selection.row == 1U)) {
        return 5;
    }
    selection = move_magic_selection_column({0U, 1U}, -1, 7U);
    if (!expect(selection.column == 0U && selection.row == 1U)) {
        return 6;
    }
    selection = move_magic_selection_row({1U, 1U}, 1, 7U);
    if (!expect(selection.column == 1U && selection.row == 0U)) {
        return 7;
    }
    selection = move_magic_selection_row({1U, 0U}, -1, 7U);
    if (!expect(selection.column == 1U && selection.row == 0U)) {
        return 8;
    }
    selection = move_magic_selection_row({0U, 0U}, 1, 6U);
    if (!expect(selection.column == 0U && selection.row == 0U)) {
        return 9;
    }

    return 0;
}
