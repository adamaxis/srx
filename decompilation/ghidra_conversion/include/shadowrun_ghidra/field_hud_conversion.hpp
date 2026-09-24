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

// ROM 0x17708: perform the field HUD render choreography. The source writes
// VDP state, streams five tile blocks, and enters the extra panel path only
// when the HUD flag is set and the selected mode is 8; host presentation is
// represented by typed callbacks instead of Genesis register writes.
FieldHudRenderResult render_field_hud(
    bool mode_panel_enabled,
    const FieldHudRenderCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
