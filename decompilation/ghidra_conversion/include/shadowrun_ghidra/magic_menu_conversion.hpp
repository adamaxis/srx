#pragma once

#include "shadowrun_ghidra/runner_record_access_conversion.hpp"

#include <cstdint>

namespace shadowrun::ghidra_conversion {

enum class MagicMenuPath {
    unavailable,
    spell_panel,
    fallback_menu,
};

struct MagicMenuSelection {
    std::uint8_t column = 0;
    std::uint8_t row = 0;
};

// ROM 0xFBA6 takes the spell panel only for the source magic-capable class
// (record +0x9B < 2) when its +0x7F magic resource is nonzero. Other records
// take the ordinary one-row menu fallback.
MagicMenuPath select_magic_menu_path(const RunnerRecordFields& record);

// ROM 0xFEBE. `first_entry_count` is the first word at the selected spell
// table entry (`record +0xA0` selects that entry). The source uses that count
// to bound the horizontal spell column for each of its two rows.
MagicMenuSelection move_magic_selection_column(
    MagicMenuSelection selection, std::int8_t delta,
    std::uint16_t first_entry_count);

// ROM 0xFF06. Rows toggle between zero and one only when the selected spell
// table entry has at least seven first-row entries; a column that is not valid
// in the second row forces the cursor back to row zero.
MagicMenuSelection move_magic_selection_row(
    MagicMenuSelection selection, std::int8_t delta,
    std::uint16_t first_entry_count);

} // namespace shadowrun::ghidra_conversion
