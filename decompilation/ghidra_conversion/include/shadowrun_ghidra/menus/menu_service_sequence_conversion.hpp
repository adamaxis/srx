#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct CommandOverviewHudPanelInput {
    // Source active record +0x3E. The ROM multiplies this word by 0x50
    // before adding the signed base tile word.
    std::uint16_t panel_variant = 0;
    // Source active record +0x2C, passed as the panel draw's D2 parameter.
    std::uint16_t field_parameter = 0;
};

struct CommandOverviewHudPanelPlan {
    std::int16_t tile_origin = 0;
    std::uint32_t destination_tile_base = 0;
    std::uint16_t field_parameter = 0;
    std::uint8_t rows = 10;
    std::uint8_t words_per_row = 8;
};

// ROM 0x110BC: the command-overview entry reads +0x3E/+0x2C, selects the
// panel tile word, and submits ten rows of eight words at D4=0xE104.
void draw_command_overview_hud_panel(
    const CommandOverviewHudPanelInput& input,
    const std::function<void(const CommandOverviewHudPanelPlan&)>& draw_panel);

struct ActiveRunnerHudPanelInput {
    // Source active record +0x3E. The three source ranges select the panel's
    // tile origin; this is a panel-layout discriminator, not a menu ID.
    std::uint16_t panel_variant = 0;
    // Caller-provided D2. The ROM 0x110E8 entry loads active record +0x2C;
    // direct 0x110EC callers prepare this parameter themselves.
    std::uint16_t field_parameter = 0;
    // Caller-provided D4. The shared entry does not initialize the panel
    // destination; its callers select the destination for their menu.
    std::uint32_t destination_tile_base = 0;
};

struct ActiveRunnerHudPanelPlan {
    std::int16_t tile_origin = 0;
    std::uint16_t field_parameter = 0;
    std::uint32_t destination_tile_base = 0;
    std::uint8_t rows = 4;
    std::uint8_t words_per_row = 4;
};

// ROM 0x110E8 -> 0x110EC: 0x110E8 loads active record +0x2C into D2 and
// enters the shared 0x110EC panel builder. 0x110EC selects the tile origin
// from +0x3E, preserves the caller's D4 destination, and submits a 4x4
// panel. The attributes/skills, field-service, and inventory/action paths
// own the surrounding menu state and share this panel layout.
void draw_active_runner_hud_panel(
    const ActiveRunnerHudPanelInput& input,
    const std::function<void(const ActiveRunnerHudPanelPlan&)>& draw_panel);

struct ActiveRunnerNameTextPlan {
    // ROM 0x1111A passes the active record's encoded name at +0x58 to the
    // common text writer.
    std::uint16_t source_record_offset = 0x58;
    std::uint16_t destination_tile_base = 0xC0A0;
    std::uint16_t palette_flags = 0x6000;
};

// ROM 0x1111A: render the selected runner's name source into the command
// overview title area. The text decoder remains the separate converted
// 0xC0DA boundary; this function only owns the source/layout selection.
void render_active_runner_name_text(
    const std::function<void(const ActiveRunnerNameTextPlan&)>& render_text);

struct MenuTextTableRef {
    // Host asset alias supplied by the caller; no ROM address is required by
    // the conversion boundary.
    std::string_view alias;
};

struct MenuTitleTextInput {
    // Source FFF0D0 selector. Values below two use the fixed Inventory
    // fallback record; values two and above index the caller's table.
    std::uint8_t selector = 0;
    MenuTextTableRef caller_table{};
};

struct MenuTitleTextPlan {
    std::string_view source_alias;
    std::size_t record_index = 0;
    std::size_t record_offset_bytes = 0;
    std::uint16_t destination_tile_base = 0xEC9C;
    std::uint16_t palette_flags = 0x6000;
    bool used_inventory_fallback = false;
};

// ROM 0x11712: select the caller-supplied 0x12-byte text record for selectors
// two and above, or the first Inventory record from 0xD1D28 for selectors
// zero and one. This models the source-table choice before 0xC0DA.
std::optional<MenuTitleTextPlan> select_menu_title_text(
    const MenuTitleTextInput& input);

// ROM 0x1170C -> 0x11712: supply the command-overview label table at
// 0xD1D3A, then apply the shared selector above.
std::optional<MenuTitleTextPlan> select_command_overview_title_text(
    std::uint8_t selector);

struct HudGlyphAttributeInput {
    // 0x03602 uses byte-sized signed branches for both comparisons. Keep the
    // source sign bit instead of turning sentinel values into large codes.
    std::int8_t action_code = 0; // source record +0x57
    std::int8_t class_code = 0;  // source record +0x9B
    std::int8_t equipped_slot = -1; // signed source record +0x56
};

// ROM 0x03602: select the palette/attribute word ORed into each menu glyph.
// The source uses the action code, class code, and signed equipped-slot state;
// it is not an empty post-panel callback.
std::uint16_t select_hud_glyph_attribute_flags(
    const HudGlyphAttributeInput& input);

// ROM 0x11136: repeat the menu row transfer seven times and return the final
// source countdown value (the source returns three after the loop).
std::uint16_t render_menu_tile_rows(const std::function<void()>& clear_row,
                                    const std::function<void()>& write_row);

// ROM 0x11442: reset the upload cursor before submitting one menu tile block.
void reset_and_upload_menu_tile_block(
    const std::function<void()>& reset_upload_state,
    const std::function<void()>& upload_tile_block);

// ROM 0x11678: submit one menu grid asset.
void write_menu_grid(const std::function<void()>& write_grid);

// ROM 0x116C4: write four menu grids, then refresh the shared menu metadata.
void write_menu_grid_quad(const std::function<void()>& write_grid,
                          const std::function<void()>& refresh_metadata);

// ROM 0x11900: prepare the menu row, then submit its repeated grid.
void prepare_menu_row_grid(const std::function<void()>& prepare_row,
                           const std::function<void()>& write_repeated_grid);

// ROM 0x11928: submit the grid before rendering the corresponding menu row.
void render_menu_row(const std::function<void()>& write_grid,
                     const std::function<void()>& render_row);

// ROM 0x11ABE: preserve the preparation result while applying the shared
// dialog service twice.
std::uint64_t prepare_menu_dialog(
    const std::function<std::uint64_t()>& prepare_dialog,
    const std::function<void()>& service_dialog);

struct MenuTextRefreshState {
    std::int8_t pending_text_count = 0;
};

// ROM 0x12062: when no text is pending, prepare and render the text twice,
// then clear the pending count.
bool refresh_menu_text(
    MenuTextRefreshState& state,
    const std::function<void()>& prepare_text,
    const std::function<void()>& render_text);

struct MenuTextCursorState {
    std::int8_t selector = 0;
    std::int16_t primary_marker = 0;
    std::int16_t secondary_marker = 0;
};

// ROM 0x1209A: alternate the two text slots until one is available, then
// render through the text boundary and run the cursor-service leaf.
void advance_menu_text_cursor(
    MenuTextCursorState& state,
    const std::function<void()>& render_text,
    const std::function<void()>& service_cursor);

struct MenuNumberRenderState {
    std::int16_t cursor = 0;
    std::uint8_t available_count = 0;
};

struct MenuNumberRenderCallbacks {
    std::function<void(std::uint16_t word)> write_number_word;
    std::function<void(std::int16_t value)> build_decimal_glyphs;
};

// ROM 0x120E4: write the number marker, advance the signed cursor with the
// source zero/overflow wrap rules, clamp past-count values, and build digits.
void render_menu_number(MenuNumberRenderState& state,
                        std::int16_t delta,
                        const MenuNumberRenderCallbacks& callbacks);

struct MenuSlotCleanupState {
    std::uint8_t slot_type = 0;
    std::uint8_t slot_subtype = 0;
    std::int8_t action_marker = 0;
    std::array<std::uint8_t, 8> pending_actions{};
    std::array<std::uint8_t, 8> active_slots{};
};

// ROM 0x1275C: clear one menu-slot pair, taking the source low-action asset
// setup branch when the shared loader sentinel is active.
void clear_menu_slot(MenuSlotCleanupState& state,
                     std::int8_t action,
                     std::size_t slot_index,
                     bool asset_load_pending,
                     const std::function<void()>& prepare_assets);

struct MenuPanelRefreshState {
    std::uint8_t toggle = 0;
};

// ROM 0x127B0: upload the panel, flip its byte toggle, update its layout, and
// submit the text render in source order.
void refresh_menu_panel(MenuPanelRefreshState& state,
                        const std::function<void()>& upload_panel,
                        const std::function<void()>& update_layout,
                        const std::function<void()>& render_text);

// ROM 0x12C1E: prepare one menu grid variant.
void prepare_menu_grid_variant_a(const std::function<void()>& prepare_grid);

// ROM 0x12C36: prepare the second menu grid variant.
void prepare_menu_grid_variant_b(const std::function<void()>& prepare_grid);

// ROM 0x12C42: prepare the third menu grid variant.
void prepare_menu_grid_variant_c(const std::function<void()>& prepare_grid);

// ROM 0x12C56: submit one repeated menu-grid write.
void write_menu_repeated_grid(const std::function<void()>& write_grid);

// ROM 0x12C5A: submit the repeated-grid write used by the neighboring menu
// service path.
void write_menu_repeated_grid_with_metadata(
    const std::function<void()>& write_grid);

// ROM 0x12CD6: submit one menu text asset.
void render_menu_text_asset(const std::function<void()>& render_text);

struct CyberwareMenuState {
    // Source 0x11950 consumes bits 14..0; bit 15 belongs to a separate
    // record concern and is intentionally not treated as an installed entry.
    std::uint16_t installed_flags = 0;
    std::uint16_t rendered_entries = 0;
};

struct CyberwareMenuCallbacks {
    std::function<void()> prepare_grid_variant_b;
    std::function<void()> prepare_row_grid;
    std::function<void()> render_title_text;
    std::function<void()> wait_for_controller_event;
    std::function<void()> render_menu_row;
};

// ROM 0x11950: prepare the cyberware panel, render its installed fifteen-bit
// entries in ascending bit order, then wait for input and render the row.
std::uint16_t run_cyberware_menu(CyberwareMenuState& state,
                                 const CyberwareMenuCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
