#include "shadowrun_ghidra/menu_service_sequence_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void draw_menu_hud_panel_variant_a(const std::function<void()>& draw_panel) {
    if (draw_panel) {
        draw_panel();
    }
}

void draw_menu_hud_panel_variant_b(const std::function<void()>& draw_panel) {
    if (draw_panel) {
        draw_panel();
    }
}

std::uint16_t render_menu_tile_rows(
    const std::function<void()>& clear_row,
    const std::function<void()>& write_row) {
    std::uint16_t countdown = 7U;
    std::uint16_t last_countdown = 0U;
    do {
        if (clear_row) {
            clear_row();
        }
        if (write_row) {
            write_row();
        }
        last_countdown = countdown;
        countdown = static_cast<std::uint16_t>(countdown - 1U);
    } while (countdown != 0xFFFFU);
    return static_cast<std::uint16_t>(last_countdown >> 1U);
}

} // namespace shadowrun::ghidra_conversion
