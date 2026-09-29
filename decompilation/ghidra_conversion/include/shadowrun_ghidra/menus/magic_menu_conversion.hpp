#pragma once

#include "shadowrun_ghidra/core/magic_spell_catalog_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_action_math_conversion.hpp"
#include "shadowrun_ghidra/session/runner_record_access_conversion.hpp"

#include <array>
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

struct MagicMenuLoopState {
    MagicMenuSelection selection{};
    std::uint16_t first_entry_count = 0;
};

struct MagicMenuLoopCallbacks {
    std::function<void()> prepare_magic_menu;
    std::function<void(const MagicMenuSelection&)> render_selection;
    std::function<void()> advance_menu_sprite_overlay;
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<void()> sample_controller_input;
    std::function<std::uint16_t()> read_sampled_input;
    std::function<void(const MagicMenuSelection&)> run_equipment_selection;
    std::function<void()> reset_cursor_animation_timer;
};

// ROM 0x105E6 reads the selected magic action and sends the current force,
// runner record, and decoded D4EA0 action row through 0x10660.
struct MagicActionEffectValueInput {
    std::uint16_t current_force = 0;
    EntityActionRecord entity{};
    MagicSpellDefinition spell{};
};

// The positive branch selects a six-tile Damage row. The host renderer uses
// the typed row and layout plan rather than ROM or VDP memory.
struct MagicActionEffectDamageGridPlan {
    std::uint16_t effect_row = 0;
    std::uint16_t source_row_stride = 0x0EU;
    std::uint16_t destination_tile_base = 0xC688U;
    std::uint16_t tile_count = 6U;
    std::uint16_t palette_flags = 0;
};

struct MagicActionEffectNoneValuePlan {
    // Source 0xC8A2C is the literal `None` value for nonpositive damage.
    std::uint16_t destination_tile_base = 0xC68CU;
};

struct MagicActionEffectValueCallbacks {
    std::function<void(const MagicActionEffectNoneValuePlan&)>
        render_none_damage_value;
    std::function<void(const MagicActionEffectDamageGridPlan&)>
        write_damage_grid;
};

struct MagicActionEffectValueResult {
    bool residual_valid = false;
    bool none_damage_value_rendered = false;
    bool damage_grid_written = false;
    std::int16_t residual_effect = 0;
    std::optional<MagicActionEffectNoneValuePlan> none_damage_value;
    std::optional<MagicActionEffectDamageGridPlan> damage_grid;
};

// ROM 0x1055E's D0=0 branch refreshes Success. Its nonzero branch refreshes
// Defense values for vs. Phys and vs. Mana using the ready-party bonus.
enum class MagicMenuValueStrip {
    success,
    defense_physical,
    defense_mana,
};

struct MagicMenuValueRefreshInput {
    bool refresh_defense_strips = false;
    std::uint16_t current_force = 0;
    std::uint8_t action_cap = 0;  // active record +0x7F
    EntityActionRecord active_entity{};
    std::array<EntityActionRecord, 3> ready_entities{};
    std::uint8_t attribute_77 = 0;
    std::uint8_t attribute_7c = 0;
};

struct MagicMenuValueGridPlan {
    MagicMenuValueStrip strip = MagicMenuValueStrip::success;
    std::uint16_t value = 0;
    std::uint16_t row = 0;
    std::uint16_t destination_tile_base = 0;
    std::uint16_t source_row_stride = 0x1AU;
    std::uint16_t tile_count = 13U;
    std::uint16_t palette_flags = 0;
    std::uint32_t glyph_source_base = 0x000C8784U;
};

constexpr std::uint32_t kMagicFallbackDefenseGlyphBase = 0x000CF610U;
constexpr std::uint16_t kMagicFallbackPhysicalDestination = 0xC51EU;
constexpr std::uint16_t kMagicFallbackManaDestination = 0xC69EU;
constexpr std::uint16_t kMagicFallbackDefensePalette = 0x2000U;

struct MagicMenuValueRefreshCallbacks {
    std::function<void()> refresh_ready_entity_animations;
    std::function<std::optional<std::uint16_t>(std::uint16_t)>
        resolve_menu_metric;
    std::function<void(const MagicMenuValueGridPlan&)> write_grid;
};

struct MagicMenuValueRefreshResult {
    bool menu_metric_available = false;
    std::uint16_t success_value = 0;
    std::uint16_t defense_physical_value = 0;
    std::uint16_t defense_mana_value = 0;
    std::vector<MagicMenuValueGridPlan> grid_requests;
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

enum class MagicForceLoopResult {
    waiting,
    adjusted,
    finished,
};

struct MagicForceSelection {
    std::uint8_t force = 1;
};

struct MagicForceState;

// ROM 0x10016 is a frame-driven controller loop. Hardware sampling and the
// presentation services are explicit host boundaries, while the loop keeps
// the source ordering and its three exit edges. A caller supplies the
// already-sampled input word; no Genesis memory or VDP state is required.
struct MagicForceSelectionLoopCallbacks {
    std::function<void()> refresh_initial_display;
    std::function<void()> advance_menu_sprite_overlay;
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<void()> sample_controller_input;
    std::function<std::uint16_t()> read_sampled_input;
    std::function<void(std::int16_t)> adjust_force;
    std::function<void()> refresh_final_display;
};

struct MagicForceDisplayCallbacks {
    std::function<void()> write_repeated_grid;
    std::function<void()> write_grid;
};

struct MagicForceAdjustmentCallbacks {
    std::function<void()> build_grouped_decimal_glyphs;
    MagicForceDisplayCallbacks display;
    std::function<void()> write_repeated_grid;
    std::function<void()> refresh_magic_menu_success_or_defense_values;
    std::function<void()> write_repeated_menu_grid;
    std::function<void()> refresh_magic_action_effect_value;
};

struct MagicEquipmentSelectionCallbacks {
    std::function<void(const MagicSpellDefinition&)> prepare_magic_action_details;
    std::function<void()> clear_entity_auxiliary_marker;
    // Return the sampled input word that caused 0x10016 to return.
    std::function<std::uint16_t()> run_magic_force_selection_loop;
    // Return the sampled input word that caused 0x10722 to return.
    std::function<std::uint16_t()> run_runner_posture_adjustment;
    std::function<void()> upload_field_asset_setup;
    std::function<void()> draw_hud_panel_triplet;
    std::function<void()> advance_entity_behavior_meter;
    std::function<void()> advance_entity_asset_meter;
};

struct MagicEquipmentSelectionResult {
    bool magic_spell_missing = false;
    bool detail_preparation_called = false;
    bool finalized = false;
};

struct MagicMenuGridPreparationCallbacks {
    std::function<void()> write_menu_repeated_grid_with_metadata;
};

struct MagicGridControlCallbacks {
    std::function<void()> write_grid;
    std::function<void(std::uint16_t)> set_display_command;
};

struct MagicSpellAvailabilityCallbacks {
    std::function<void()> write_repeated_grid;
    std::function<void(const MagicSpellDefinition&)> render_spell_text;
    std::function<void()> write_grid;
};

enum class MagicActionTextRegion {
    set_force_and_posture_heading,
    success_label,
    force_label,
};

struct MagicActionDetailCallbacks {
    std::function<void()> prepare_menu_grid_then_service;
    std::function<void(MagicActionTextRegion region)> render_action_text;
    std::function<void()> adjust_runner_posture;
    std::function<void(const MagicSpellDefinition&)> render_spell_availability;
    std::function<void()> build_grouped_decimal_glyphs;
};

enum class MagicSpellRecordTextRegion {
    spell_name,
    spell_description,
};

struct MagicMenuSelectionDisplayCallbacks {
    std::function<void()> prepare_menu_grid_then_service;
    std::function<void()> refresh_force_display;
    std::function<void(std::uint8_t spell_code, std::uint16_t tile_mask)>
        transfer_spell_glyph_triple;
    std::function<void(const MagicSpellDefinition& spell,
                       MagicSpellRecordTextRegion region)>
        render_spell_record_text;
    std::function<void(MagicActionTextRegion region)> render_action_text;
    std::function<void()> adjust_runner_posture;
    std::function<void(const MagicSpellDefinition&)> render_spell_availability;
    std::function<void()> build_grouped_decimal_glyphs;
};

// ROM 0xFBA6 takes the spell panel only for the source magic-capable class
// (record +0x9B < 2) when its +0x7F magic resource is nonzero. Other records
// take the ordinary one-row menu fallback.
MagicMenuPath select_magic_menu_path(const RunnerRecordFields& record);

// ROM 0xFBA6's spell-panel interaction loop. Navigation has source priority
// over action and exit edges; each navigation redraw resets the cursor-sprite
// countdown, action redraw does not, and bit 7 exits only after those checks.
std::uint16_t run_magic_menu_loop(
    MagicMenuLoopState& state, const MagicMenuLoopCallbacks& callbacks = {});

// ROM 0x105E6: refresh the selected magic action's Damage value. Nonpositive
// residuals render the literal None value; positive residuals select source
// damage row (residual - 1) and submit its six-tile grid.
MagicActionEffectValueResult refresh_magic_action_effect_value(
    const MagicActionEffectValueInput& input,
    const MagicActionEffectValueCallbacks& callbacks = {});

// ROM 0x1055E: refresh the magic menu's Success strip when D0=0, or its
// ready-party Defense strips (vs. Phys and vs. Mana) when D0 is nonzero.
MagicMenuValueRefreshResult refresh_magic_menu_success_or_defense_values(
    const MagicMenuValueRefreshInput& input,
    const MagicMenuValueRefreshCallbacks& callbacks = {});

// ROM 0x106DA: 0x1047E bonus plus record +0x77 (D2==0, dest 0xC51E) or +0x7C
// (D2!=0, dest 0xC69E). Glyph row is 0 when the sum is 0, otherwise sum-1,
// at 0xCF610 stride 0x1A. D7=0xC, D1=0, palette 0x2000. 0x1232 stays a
// write callback.
MagicMenuValueGridPlan prepare_magic_fallback_defense_strip(
    const std::array<EntityActionRecord, 3>& ready_entities,
    const EntityActionRecord& actor,
    bool mana_strip);

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

// Native conversion boundary: use the typed US catalog for runtime-facing
// spell selection. The byte-vector overload above remains an explicit
// source-table/reference helper for ROM audit tests.
std::optional<MagicSpellSelection> select_magic_spell(
    const std::vector<std::uint8_t>& spell_table,
    MagicMenuSelection selection,
    const UsMagicSpellCatalog& spell_catalog);

// Native conversion boundary: the +0xA0-selected runner row is already a
// typed list of spell/force entries, so no ROM table bytes are needed here.
std::optional<MagicSpellSelection> select_magic_spell(
    const MagicSpellTable& spell_table,
    MagicMenuSelection selection,
    const UsMagicSpellCatalog& spell_catalog);

// Read the source signed availability byte directly from one 0x30-byte spell
// record. This is an offline/reference conversion helper; the runtime does
// not carry the ROM catalog or perform address arithmetic.
std::optional<std::int8_t> read_magic_spell_availability(
    const std::vector<std::uint8_t>& spell_records,
    std::uint8_t spell_code);

// ROM 0x100A8 clamps force to 1..table_parameter and publishes +0x9D.
std::uint8_t clamp_magic_force(std::uint8_t current, std::int8_t delta,
                               std::uint8_t table_parameter);

// ROM 0xFF5A: prepare the magic detail panel, carry the selected force into
// an already-equipped item's rounds when the source +0x56/+0x57 gate allows
// it, install spell code + 1, route the force/posture input exits, then clear
// the inventory-slot marker and publish the field/HUD updates.
MagicEquipmentSelectionResult run_magic_equipment_selection(
    RunnerRecordFields& record, MagicForceState& force_state,
    std::uint8_t spell_code,
    const MagicEquipmentSelectionCallbacks& callbacks = {});

// ROM 0x10016's input boundary: B raises force, A lowers it, and the source
// confirm/cancel edges leave the force/posture loop. Bit D is deliberately not
// an exit edge; it returns to the next frame. The redraw service remains a
// separate host boundary.
MagicForceLoopResult apply_magic_force_input(
    MagicForceSelection& state, std::uint16_t input,
    std::uint8_t table_parameter);

// ROM 0x10016: initial force display, overlay/frame/input services, B/A
// adjustment priority, and the bit-9/E/C exit redraw. The host controller
// owns the event loop; this function is the source behavior for one bounded
// selection interaction and returns only on one of the source exit edges.
std::uint16_t run_magic_force_selection_loop(
    const MagicForceSelectionLoopCallbacks& callbacks = {});

// ROM 0x10172: refresh the force meter in the exact source order. The
// repeated-grid and normal-grid operations are host presentation boundaries;
// the conditionals are source behavior and therefore stay in this contract.
void refresh_magic_force_display(
    std::uint16_t current_force, std::uint16_t maximum_force,
    const MagicForceDisplayCallbacks& callbacks = {});

// ROM 0x100A8: update the typed current force, publish record +0x9D, then
// preserve the source callback order. The combat refresh is allowed to alter
// the maximum-force state through its host boundary: the source saves only
// current force (+0xFFF096), restores that word, and leaves maximum force
// (+0xFFF098) as refreshed.
struct MagicForceState {
    std::uint16_t current_force = 1;
    std::uint16_t maximum_force = 1;
};

// ROM 0x101FE: the selected table word supplies the force maximum. The
// current force survives only while the source record has no inventory slot
// and still carries the selected spell item code; every other selection
// starts at that spell's maximum.
void select_magic_force(MagicForceState& state,
                        const MagicSpellSelection& selection,
                        const RunnerRecordFields& record);

// ROM 0x10260: submit the magic menu's metadata grid, then select the current
// and maximum force for the cursor's spell entry.
void prepare_magic_menu_grid_and_select_force(
    MagicForceState& state, const MagicSpellSelection& selection,
    const RunnerRecordFields& record,
    const MagicMenuGridPreparationCallbacks& callbacks = {});

void adjust_magic_force(
    RunnerRecordFields& record, MagicForceState& state, std::int16_t delta,
    const MagicForceAdjustmentCallbacks& callbacks = {});

// ROM 0x102D4: prepare the spell detail panel. The text renderer is called
// three times by the source, not once with an inferred repeat count.
void prepare_magic_action_details(
    const MagicSpellDefinition& spell,
    const MagicActionDetailCallbacks& callbacks = {});

// ROM 0x10276 enters the shared magic-detail tail at 0x102EC. It prepares
// the selected spell's force display, transfers the spell glyph triple with
// the source class mask, renders the spell name/description, then emits the
// shared Success/Force labels, posture, availability, and force value.
void render_magic_menu_selection_display(
    const MagicSpellDefinition& spell,
    const MagicMenuSelectionDisplayCallbacks& callbacks = {});

// ROM 0x1088C: a negative signed +0x25 spell byte draws the unavailable
// marker; otherwise the source renders the selected spell's text and one
// grid. The runtime receives the typed D4EA0 row, not a detached byte.
void render_magic_spell_availability(
    const MagicSpellDefinition& spell,
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
