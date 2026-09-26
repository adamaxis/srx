#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

// ROM 0x110BC: menu path A submits one HUD-panel draw operation.
void draw_menu_hud_panel_variant_a(const std::function<void()>& draw_panel);

struct ActiveRunnerHudPanelInput {
    // Source active record +0x3E. The three source ranges select the panel's
    // tile origin; this is a panel-layout discriminator, not a menu ID.
    std::int16_t panel_variant = 0;
    // Source active record +0x2C, passed as the panel draw's D2 parameter.
    std::uint16_t field_parameter = 0;
};

struct ActiveRunnerHudPanelPlan {
    std::int16_t tile_origin = 0;
    std::uint16_t field_parameter = 0;
    std::uint8_t rows = 4;
    std::uint8_t words_per_row = 4;
};

// ROM 0x110EC: choose the active runner HUD panel tile origin from +0x3E,
// pass +0x2C through as the field parameter, and submit a 4x4 panel. The
// attributes/skills, field-service, and inventory/action paths own the
// surrounding menu state and share this panel layout.
void draw_active_runner_hud_panel(
    const ActiveRunnerHudPanelInput& input,
    const std::function<void(const ActiveRunnerHudPanelPlan&)>& draw_panel);

struct HudGlyphAttributeInput {
    std::uint8_t action_code = 0; // source record +0x57
    std::uint8_t class_code = 0;  // source record +0x9B
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
