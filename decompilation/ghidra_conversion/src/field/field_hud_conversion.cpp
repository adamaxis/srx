#include "shadowrun_ghidra/field/field_hud_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void upload_field_asset_sequence(
    const FieldHudAssetTransferCallbacks& callbacks) {
    if (callbacks.write_wrapped_transfer) {
        callbacks.write_wrapped_transfer();
    }
    if (callbacks.write_grid) {
        for (int grid = 0; grid < 5; ++grid) {
            callbacks.write_grid();
        }
    }
}

void upload_field_asset_setup(
    const FieldHudAssetTransferCallbacks& callbacks) {
    if (callbacks.write_wrapped_transfer) {
        for (int group = 0; group < 2; ++group) {
            for (int transfer = 0; transfer < 14; ++transfer) {
                callbacks.write_wrapped_transfer();
            }
        }
    }
    if (callbacks.draw_hud_panel_triplet) {
        callbacks.draw_hud_panel_triplet();
    }
}

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

void draw_hud_panel_triplet(const std::function<void()>& draw_panel) {
    if (!draw_panel) {
        return;
    }
    for (int pass = 0; pass < 3; ++pass) {
        draw_panel();
    }
}

} // namespace shadowrun::ghidra_conversion
