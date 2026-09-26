#include "shadowrun_ghidra/field/field_viewport_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void render_normal_field_viewport(
    FieldViewportState& state,
    const FieldViewportRenderBoundary& render_viewport) {
    state.normal_render_pending = 0U;
    if (render_viewport) {
        render_viewport();
    }
}

void render_alternate_field_viewport(
    FieldViewportState& state,
    const FieldViewportRenderBoundary& render_viewport) {
    state.alternate_render_pending = 0U;
    state.render_mask = 0x8000U;
    if (render_viewport) {
        render_viewport();
    }
    state.render_mask = 0U;
}

} // namespace shadowrun::ghidra_conversion
