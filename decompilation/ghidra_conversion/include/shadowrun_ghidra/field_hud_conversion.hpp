#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct FieldHudRenderCallbacks {
    std::function<void()> prepare_display;
    std::function<void()> render_sprite_stream;
    std::function<void()> upload_tile_block;
    std::function<void()> refresh_hud_state;
    std::function<std::uint8_t()> select_hud_mode;
    std::function<void()> draw_mode_panel;
    std::function<void()> advance_mode_panel;
    std::function<void()> draw_mode_footer;
};

struct FieldHudRenderResult {
    std::uint32_t uploaded_tile_blocks = 0;
    bool mode_panel_rendered = false;
};

struct FieldHudAssetTransferCallbacks {
    std::function<void()> write_wrapped_transfer;
    std::function<void()> write_grid;
    std::function<void()> draw_hud_panel_triplet;
};

// ROM 0x334C: emit one wrapped transfer followed by five grid writes.
void upload_field_asset_sequence(
    const FieldHudAssetTransferCallbacks& callbacks);

// ROM 0x3404: emit two groups of fourteen wrapped transfers, then draw the
// three HUD panels.
void upload_field_asset_setup(
    const FieldHudAssetTransferCallbacks& callbacks);

// ROM 0x17708: perform the field HUD render choreography. The source writes
// VDP state, streams five tile blocks, and enters the extra panel path only
// when the HUD flag is set and the selected mode is 8; host presentation is
// represented by typed callbacks instead of Genesis register writes.
FieldHudRenderResult render_field_hud(
    bool mode_panel_enabled,
    const FieldHudRenderCallbacks& callbacks);

// ROM 0x3548: invoke the source HUD-panel drawing leaf three times.
void draw_hud_panel_triplet(const std::function<void()>& draw_panel);

} // namespace shadowrun::ghidra_conversion
