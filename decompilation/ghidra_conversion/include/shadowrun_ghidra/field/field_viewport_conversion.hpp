#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct FieldViewportState {
    std::uint16_t normal_render_pending = 0;
    std::uint16_t alternate_render_pending = 0;
    std::uint16_t render_mask = 0;
};

using FieldViewportRenderBoundary = std::function<void()>;

// ROM 0x2C92: clear the normal viewport-render request and run the shared
// field viewport renderer.
void render_normal_field_viewport(
    FieldViewportState& state,
    const FieldViewportRenderBoundary& render_viewport);

// ROM 0x2CB2: clear the alternate viewport-render request, render with the
// source 0x8000 mask, then restore the ordinary mask.
void render_alternate_field_viewport(
    FieldViewportState& state,
    const FieldViewportRenderBoundary& render_viewport);

} // namespace shadowrun::ghidra_conversion
