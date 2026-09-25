#pragma once

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct FieldModeSelectionGlyphRecord {
    std::int8_t marker = 0;
};

struct FieldModeSelectionPresentationState {
    std::uint8_t menu_mode = 0; // FFF0B6
    std::int8_t mode_result = 0; // FFF0B7
    std::int8_t primary_cursor = 0; // FFF0D0
    std::int8_t saved_selection = 0; // FFF0B8
    std::array<FieldModeSelectionGlyphRecord, 8> glyph_records{};
};

enum class MenuResourceFamily {
    general_menu_records,
    menu_mode_six_records,
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
};

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

// ROM 0x58B1E: write six grids followed by two more grids.
void render_field_mode_grid_pair_batches(
    const FieldModeSelectionPresentationCallbacks& callbacks);

// ROM 0x587DA: save the selected cursor and choose the field-service
// presentation branch from the signed result, primary cursor, and the
// resource family selected by FFF0B6.
void render_field_mode_selection_presentation(
    FieldModeSelectionPresentationState& state,
    const FieldModeSelectionPresentationCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
