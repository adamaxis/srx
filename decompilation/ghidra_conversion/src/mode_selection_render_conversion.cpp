#include "shadowrun_ghidra/mode_selection_render_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void render_mode_selection_panels(
    ModeSelectionRenderState& state,
    const ModeSelectionRenderCallbacks& callbacks) {
    for (std::uint8_t panel = 0; panel < kModeSelectionPanelCount; ++panel) {
        if (callbacks.prepare_panel) {
            callbacks.prepare_panel(panel);
        }
        if (callbacks.upload_panel) {
            callbacks.upload_panel(panel);
        }
        for (std::uint8_t pass = 0; pass < kModeSelectionVblankPasses;
             ++pass) {
            if (callbacks.wait_for_vblank) {
                callbacks.wait_for_vblank();
            }
            if (callbacks.latch_controller_events) {
                callbacks.latch_controller_events();
            }
            if (callbacks.service_world_state) {
                callbacks.service_world_state();
            }
        }
        state.rendered_panel_count = static_cast<std::uint8_t>(panel + 1U);
    }
    if (callbacks.render_title_text) {
        callbacks.render_title_text();
    }
    if (callbacks.read_final_text_marker) {
        state.final_text_marker = callbacks.read_final_text_marker();
    }
}

} // namespace shadowrun::ghidra_conversion
