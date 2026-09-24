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

} // namespace shadowrun::ghidra_conversion
