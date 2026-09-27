#pragma once

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct FieldModeSelectionGlyphRecord {
    std::int8_t marker = 0;
};

enum class FieldModeGlyphPalettePlane {
    low_0x2000,
    high_0x6000,
};

struct FieldModeRecordPresentationPlan {
    std::uint8_t marker = 0;
    bool writes_grid = false;
    FieldModeGlyphPalettePlane glyph_palette =
        FieldModeGlyphPalettePlane::high_0x6000;
};

struct FieldModeInventoryGlyphPlan {
    std::uint8_t item_code = 0;
    std::uint8_t item_definition_index = 0;
    bool uses_blank_asset = true;
    // 0x58AA4 overwrites the provisional palette choice with 0x6000
    // immediately before transfer_menu_glyph_triple.
    FieldModeGlyphPalettePlane glyph_palette =
        FieldModeGlyphPalettePlane::high_0x6000;
};

struct FieldModeSelectionPresentationState {
    std::uint8_t menu_mode = 0; // FFF0B6
    std::int8_t mode_result = 0; // FFF0B7
    std::int8_t primary_cursor = 0; // FFF0D0
    std::int8_t saved_selection = 0; // FFF0B8
    // Source bases 0x1AAC86, 0x1AADBF, and 0x1AAD77 are distinct eight-
    // record tables for negative-result cursors 0, 1, and 2.
    std::array<std::array<FieldModeSelectionGlyphRecord, 8>, 3>
        negative_glyph_records{};
    std::array<FieldModeSelectionGlyphRecord, 8> glyph_records{};
    // Eight bytes read from the active runner record at +0x66..+0x6D by
    // 0x58AA4. They are item codes, not already-resolved icon indices.
    std::array<std::uint8_t, 8> inventory_item_codes{};
};

enum class MenuResourceFamily {
    general_menu_records,
    menu_mode_six_records,
};

enum class FieldModeNegativeGlyphTable {
    cursor_zero,
    cursor_one,
    cursor_two,
};

enum class FieldModeActionRecordTable {
    fixed_low_menu,
    fixed_selection_one,
    dynamic_menu_mode,
};

struct MenuResourceRecord {
    MenuResourceFamily family = MenuResourceFamily::general_menu_records;
    std::uint16_t record_offset = 0;
};

struct MenuResourceEntry {
    MenuResourceRecord resource{};
    std::uint16_t word_offset = 0;
};

struct FieldModeSelectionPresentationCallbacks {
    std::function<void()> write_grid;
    std::function<void()> transfer_menu_glyph_triple;
    std::function<void()> render_encoded_text;
    std::function<void(const MenuResourceRecord&)> select_resource;
    // Expose the source data-table choice before the shared marker/glyph
    // loops. The caller supplies the corresponding typed row in state.
    std::function<void(FieldModeNegativeGlyphTable, std::uint8_t)>
        select_negative_glyph_table;
    std::function<void(FieldModeActionRecordTable, std::int16_t)>
        select_action_record_table;
    // Optional detailed boundary for the source's 0x58AA4 item-icon loop.
    // Existing callers may keep using transfer_menu_glyph_triple.
    std::function<void(std::size_t, const FieldModeInventoryGlyphPlan&)>
        transfer_inventory_glyph;
    // Optional detailed boundary for the source's 0x58A2E marker loop.
    std::function<void(std::size_t, const FieldModeRecordPresentationPlan&)>
        transfer_record_glyph;
    std::function<void(std::size_t, const FieldModeRecordPresentationPlan&)>
        write_record_grid;
};

// ROM 0x58A2E: preserve the marker test, the 'E' grid sentinel, and the
// source palette split used by non-sentinel glyph records.
FieldModeRecordPresentationPlan plan_field_mode_record(
    std::uint8_t marker);

// ROM 0x58AA4: resolve one active-runner item byte to the extracted item
// definition row consumed by the glyph transfer. Zero is the blank asset.
FieldModeInventoryGlyphPlan plan_field_mode_inventory_glyph(
    std::uint8_t item_code);

// ROM 0x5763C: choose the menu-mode-six table when FFF0B6 == 6, otherwise
// choose the general table, then select the saved cursor's 0x10-byte record.
MenuResourceRecord select_menu_resource(
    std::uint8_t menu_mode,
    std::int8_t saved_selection);

// ROM 0x57628: select the saved resource record, then advance by two bytes
// per primary cursor position to the word consumed by the action path.
MenuResourceEntry select_menu_resource_entry(
    std::uint8_t menu_mode,
    std::int8_t saved_selection,
    std::int8_t primary_cursor);

// ROM 0x58A2E: inspect eight three-byte glyph records. Only a record whose
// signed leading marker is exactly 'E' takes the grid path.
void render_alternating_field_mode_records(
    const std::array<FieldModeSelectionGlyphRecord, 8>& records,
    const FieldModeSelectionPresentationCallbacks& callbacks);

// ROM 0x58AA4: transfer eight glyph triples and add the result-4 text pass.
void render_field_mode_glyph_row(
    std::int8_t saved_selection,
    const FieldModeSelectionPresentationCallbacks& callbacks);

// Source-faithful overload for 0x58AA4. The compatibility overload above
// supplies eight blank item bytes for callers that only need the old loop.
void render_field_mode_glyph_row(
    const std::array<std::uint8_t, 8>& inventory_item_codes,
    std::int8_t saved_selection,
    const FieldModeSelectionPresentationCallbacks& callbacks);

// ROM 0x58B1E: write six grids followed by two more grids.
void render_field_mode_grid_pair_batches(
    const FieldModeSelectionPresentationCallbacks& callbacks);

// ROM 0x587DA: save the selected cursor and choose the field-service
// presentation branch from the signed result, wrapped byte-flag cursor
// comparisons, and the resource family selected by FFF0B6. The zero-result
// cursor-0 path selects the dynamic action table; it is not the glyph row.
void render_field_mode_selection_presentation(
    FieldModeSelectionPresentationState& state,
    const FieldModeSelectionPresentationCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
