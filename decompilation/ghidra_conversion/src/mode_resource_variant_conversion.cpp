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

void prepare_mode_resource_variant_a(
    ModeResourceVariantState& state,
    const std::function<void()>& write_wrapped_word_block) {
    for (unsigned int transfer = 0; transfer < 0x50U; ++transfer) {
        if (write_wrapped_word_block) {
            write_wrapped_word_block();
        }
        ++state.wrapped_transfer_count;
    }
}

} // namespace shadowrun::ghidra_conversion
