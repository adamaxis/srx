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

std::optional<std::int8_t> read_magic_spell_availability(
    const std::vector<std::uint8_t>& spell_records,
    const std::uint8_t spell_code) {
    const auto record_offset = static_cast<std::size_t>(spell_code) *
                               kMagicSpellRecordSize;
    constexpr std::size_t kAvailabilityOffset = 0x25U;
    if (record_offset + kAvailabilityOffset >= spell_records.size()) {
        return std::nullopt;
    }
    return static_cast<std::int8_t>(spell_records[record_offset +
                                                  kAvailabilityOffset]);
}

std::optional<MagicSpellSelection> select_magic_spell(
    const std::vector<std::uint8_t>& spell_table,
    const MagicMenuSelection selection,
    const std::vector<std::uint8_t>& spell_records) {
    if (spell_table.size() < 2U || selection.row > 1U ||
        selection.column >= 7U) {
        return std::nullopt;
    }
    const auto count = static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(spell_table[0]) << 8U) |
        spell_table[1]);
    const auto entry_index = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(selection.row) * 7U + selection.column);
    if (entry_index >= count) {
        return std::nullopt;
    }
    const auto entry_offset = static_cast<std::size_t>(2U) +
                              static_cast<std::size_t>(entry_index) * 2U;
    if (entry_offset + 1U >= spell_table.size()) {
        return std::nullopt;
    }

    const auto availability =
        read_magic_spell_availability(spell_records, spell_table[entry_offset]);
    if (!availability.has_value()) {
        return std::nullopt;
    }
    MagicSpellSelection result{};
    result.table_entry_index = entry_index;
    result.spell_code = spell_table[entry_offset];
    result.table_parameter = spell_table[entry_offset + 1U];
    result.availability_byte = *availability;
    result.availability = *availability >= 0
                              ? MagicSpellAvailability::available
                              : MagicSpellAvailability::unavailable;
    return result;
}

} // namespace shadowrun::ghidra_conversion
