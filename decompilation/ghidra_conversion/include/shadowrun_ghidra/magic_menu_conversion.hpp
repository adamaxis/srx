#pragma once

#include "shadowrun_ghidra/runner_record_access_conversion.hpp"

#include <cstdint>
#include <optional>
#include <vector>

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

constexpr std::size_t kMagicSpellRecordSize = 0x30U;

enum class MagicSpellAvailability {
    available,
    unavailable,
};

struct MagicSpellSelection {
    std::uint16_t table_entry_index = 0;
    std::uint8_t spell_code = 0;
    // The low byte of the table word is consumed by the source force/value
    // selection path and must not be discarded as padding.
    std::uint8_t table_parameter = 0;
    std::int8_t availability_byte = -1;
    MagicSpellAvailability availability = MagicSpellAvailability::unavailable;
};

enum class MagicActionResult {
    prepared,
    unavailable_spell,
};

struct MagicActionPreparation {
    MagicActionResult result = MagicActionResult::unavailable_spell;
    std::uint8_t spell_item_code = 0;
    std::uint8_t force = 0;
    // ROM 0x125FA clears an auxiliary entity marker when an ordinary
    // equipped item was active. The marker itself remains an external
    // boundary, but the source condition is returned to the caller.
    bool clear_active_item_marker = false;
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

// ROM 0xFC68..0xFC7C selects the runner's table from FFF9FA using the
// big-endian +0xA0 table index. Each table begins with a word count followed
// by two-byte entries; 0x1088C uses the entry high byte as a D4EA0 spell
// record index and gates the detail panel on record byte +0x25.
std::optional<MagicSpellSelection> select_magic_spell(
    const std::vector<std::uint8_t>& spell_table,
    MagicMenuSelection selection,
    const std::vector<std::uint8_t>& spell_records);

// Read the source signed availability byte directly from one 0x30-byte spell
// record. This is an offline/reference conversion helper; the runtime does
// not carry the ROM catalog or perform address arithmetic.
std::optional<std::int8_t> read_magic_spell_availability(
    const std::vector<std::uint8_t>& spell_records,
    std::uint8_t spell_code);

// ROM 0x100A8 clamps force to 1..table_parameter and publishes +0x9D.
std::uint8_t clamp_magic_force(std::uint8_t current, std::int8_t delta,
                               std::uint8_t table_parameter);

// ROM 0xFF5A's source mutation after the force/posture selection boundary.
// The target/effect loop at 0x10016 remains outside this preparation helper.
MagicActionPreparation prepare_magic_action(
    RunnerRecordFields& record, const MagicSpellSelection& selection,
    std::uint8_t force);

} // namespace shadowrun::ghidra_conversion
