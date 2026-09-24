#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct ModeExitResourceState {
    bool optional_resource_pending = false;
    std::int8_t asset_mode = 0;
    std::int8_t resource_cursor = 0;
    bool optional_resource_cleared = false;
    bool hud_rendered = false;
};

struct ModeExitResourceCallbacks {
    std::function<void()> prepare_optional_resource;
    std::function<void()> refresh_mode_resource_metadata;
    std::function<void()> decode_and_upload_source_stream;
    std::function<void()> prepare_mode_resource_variant_a;
    std::function<void()> prepare_mode_resource_variant_b;
    std::function<void()> render_hud_sprite_stream;
};

// ROM 0x1F292: prepare mode resources, decode the source stream, and choose
// between the alternate resource path and the ordinary HUD render path.
void prepare_mode_exit_resources(
    ModeExitResourceState& state,
    const ModeExitResourceCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
