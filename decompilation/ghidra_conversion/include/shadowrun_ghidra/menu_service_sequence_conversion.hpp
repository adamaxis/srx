#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

// ROM 0x110BC: menu path A submits one HUD-panel draw operation.
void draw_menu_hud_panel_variant_a(const std::function<void()>& draw_panel);

// ROM 0x110EC: menu path B submits one HUD-panel draw operation.
void draw_menu_hud_panel_variant_b(const std::function<void()>& draw_panel);

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

} // namespace shadowrun::ghidra_conversion
