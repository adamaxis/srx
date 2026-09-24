#include "shadowrun_ghidra/field_hud_conversion.hpp"

namespace shadowrun::ghidra_conversion {

FieldHudRenderResult render_field_hud(
    const bool mode_panel_enabled,
    const FieldHudRenderCallbacks& callbacks) {
    FieldHudRenderResult result{};
    if (callbacks.prepare_display) {
        callbacks.prepare_display();
    }
    if (callbacks.render_sprite_stream) {
        callbacks.render_sprite_stream();
    }
    if (callbacks.upload_tile_block) {
        for (std::uint32_t block = 0U; block < 5U; ++block) {
            callbacks.upload_tile_block();
            ++result.uploaded_tile_blocks;
        }
    }
    if (callbacks.refresh_hud_state) {
        callbacks.refresh_hud_state();
    }

    if (mode_panel_enabled && callbacks.select_hud_mode &&
        callbacks.select_hud_mode() == 8U) {
        if (callbacks.draw_mode_panel) {
            callbacks.draw_mode_panel();
        }
        if (callbacks.advance_mode_panel) {
            callbacks.advance_mode_panel();
        }
        if (callbacks.draw_mode_footer) {
            callbacks.draw_mode_footer();
        }
        result.mode_panel_rendered = true;
    }
    return result;
}

} // namespace shadowrun::ghidra_conversion
