#pragma once

#include "shadowrun_ghidra/session/runner_record_access_conversion.hpp"

#include <cstdint>
#include <functional>
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

enum class MagicForceLoopResult {
    waiting,
    adjusted,
    finished,
};

struct MagicForceSelection {
    std::uint8_t force = 1;
};

struct MagicForceDisplayCallbacks {
    std::function<void()> write_repeated_grid;
    std::function<void()> write_grid;
};

struct MagicForceAdjustmentCallbacks {
    std::function<void()> build_grouped_decimal_glyphs;
    MagicForceDisplayCallbacks display;
    std::function<void()> write_repeated_grid;
    std::function<void()> refresh_equipment_combat_values;
    std::function<void()> write_repeated_menu_grid;
    std::function<void()> refresh_equipment_action_metric;
};

struct MagicGridControlCallbacks {
    std::function<void()> write_grid;
    std::function<void(std::uint16_t)> set_display_command;
};

struct MagicSpellAvailabilityCallbacks {
    std::function<void()> write_repeated_grid;
    std::function<void()> render_spell_text;
    std::function<void()> write_grid;
};

struct MagicActionDetailCallbacks {
    std::function<void()> prepare_menu_grid_then_service;
    std::function<void()> render_spell_text;
    std::function<void()> adjust_runner_posture;
    std::function<void()> render_spell_availability;
    std::function<void()> build_grouped_decimal_glyphs;
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

// ROM 0x10016: B raises force, A lowers it, and the source confirm/cancel
// edges leave the force/posture loop. This models the input/state boundary;
// the source's VDP redraw helpers remain outside the conversion.
MagicForceLoopResult apply_magic_force_input(
    MagicForceSelection& state, std::uint16_t input,
    std::uint8_t table_parameter);

// ROM 0x10172: refresh the force meter in the exact source order. The
// repeated-grid and normal-grid operations are host presentation boundaries;
// the conditionals are source behavior and therefore stay in this contract.
void refresh_magic_force_display(
    std::uint16_t current_force, std::uint16_t maximum_force,
    const MagicForceDisplayCallbacks& callbacks = {});

// ROM 0x100A8: update the typed current force, publish record +0x9D, then
// preserve the source callback order. The combat refresh is allowed to alter
// the force state through its host boundary, so the source's saved
// current/maximum pair is restored before the final menu refreshes.
struct MagicForceState {
    std::uint16_t current_force = 1;
    std::uint16_t maximum_force = 1;
};

void adjust_magic_force(
    RunnerRecordFields& record, MagicForceState& state, std::int16_t delta,
    const MagicForceAdjustmentCallbacks& callbacks = {});

// ROM 0x102D4: prepare the spell detail panel. The text renderer is called
// three times by the source, not once with an inferred repeat count.
void prepare_magic_action_details(
    const MagicActionDetailCallbacks& callbacks = {});

// ROM 0x1088C: a negative signed availability byte draws the unavailable
// marker; otherwise the source renders the spell text and one grid.
void render_magic_spell_availability(
    std::int8_t availability_byte,
    const MagicSpellAvailabilityCallbacks& callbacks = {});

// ROM 0x1098E: clear nine menu grids, optionally clear the tenth source grid,
// then select the normal display command 0x8174.
void clear_magic_menu_grids(
    bool include_extra_grid,
    const MagicGridControlCallbacks& callbacks = {});

// ROM 0x10A82: select command 0x8134, write four grids, then restore 0x8174.
void refresh_menu_grid_quadruple(
    const MagicGridControlCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
