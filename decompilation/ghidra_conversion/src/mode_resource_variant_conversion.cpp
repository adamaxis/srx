#include "shadowrun_ghidra/mode_resource_variant_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void prepare_mode_resource_variant_b(
    ModeResourceVariantState& state,
    const std::function<void()>& refresh_resource_state,
    const std::function<void()>& draw_hud_panel) {
    if (refresh_resource_state) {
        refresh_resource_state();
    }
    state.resource_state_refreshed = true;
    if (draw_hud_panel) {
        draw_hud_panel();
    }
    state.hud_panel_requested = true;
}

} // namespace shadowrun::ghidra_conversion
