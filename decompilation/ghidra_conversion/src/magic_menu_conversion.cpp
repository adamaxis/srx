#include "shadowrun_ghidra/magic_menu_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {
namespace {

std::int16_t maximum_magic_column(const std::uint8_t row,
                                  const std::uint16_t first_entry_count) {
    auto maximum = static_cast<std::int16_t>(first_entry_count);
    if (row == 0U) {
        if (maximum >= 7) {
            maximum = 6;
        }
    } else {
        maximum = static_cast<std::int16_t>(maximum - 7);
    }
    return maximum;
}

} // namespace

MagicMenuPath select_magic_menu_path(const RunnerRecordFields& record) {
    if (record.archetype_index < 2U && record.auxiliary[0] != 0U) {
        return MagicMenuPath::spell_panel;
    }
    return MagicMenuPath::fallback_menu;
}

MagicMenuSelection move_magic_selection_column(
    MagicMenuSelection selection, const std::int8_t delta,
    const std::uint16_t first_entry_count) {
    const auto maximum = maximum_magic_column(selection.row,
                                              first_entry_count);
    const auto next = static_cast<std::int16_t>(selection.column) + delta;
    if (next < 0) {
        selection.column = static_cast<std::uint8_t>(
            std::max<std::int16_t>(0, maximum));
    } else if (next <= maximum) {
        selection.column = static_cast<std::uint8_t>(next);
    } else {
        selection.column = 0U;
    }
    return selection;
}

MagicMenuSelection move_magic_selection_row(
    MagicMenuSelection selection, const std::int8_t delta,
    const std::uint16_t first_entry_count) {
    if (static_cast<std::int16_t>(first_entry_count) - 7 < 0) {
        return selection;
    }

    const auto next = static_cast<std::int16_t>(selection.row) + delta;
    if (next < 0) {
        selection.row = 1U;
    } else if (next == 2) {
        selection.row = 0U;
    } else {
        selection.row = static_cast<std::uint8_t>(next);
    }

    if (maximum_magic_column(selection.row, first_entry_count) <
        static_cast<std::int16_t>(selection.column)) {
        selection.row = 0U;
    }
    return selection;
}

} // namespace shadowrun::ghidra_conversion
