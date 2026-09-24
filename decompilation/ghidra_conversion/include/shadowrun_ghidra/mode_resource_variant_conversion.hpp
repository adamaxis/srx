#pragma once

#include <functional>

namespace shadowrun::ghidra_conversion {

struct ModeResourceVariantState {
    bool resource_state_refreshed = false;
    bool hud_panel_requested = false;
    unsigned int wrapped_transfer_count = 0;
};

// ROM 0x1F34A: refresh the mode resource state, then draw the HUD panel.
void prepare_mode_resource_variant_b(
    ModeResourceVariantState& state,
    const std::function<void()>& refresh_resource_state,
    const std::function<void()>& draw_hud_panel);

// ROM 0x1F38A: issue 80 repeated source-wrapped word-block transfers.
void prepare_mode_resource_variant_a(
    ModeResourceVariantState& state,
    const std::function<void()>& write_wrapped_word_block);

} // namespace shadowrun::ghidra_conversion
