#include "shadowrun_ghidra/menu_state_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void initialize_menu_slot_states(std::array<MenuSlotState, 5>& slots) {
    for (auto& slot : slots) {
        slot.status = 6U;
        slot.timer = 1U;
    }
}

void render_menu_footer(
    MenuFooterRenderState& state,
    const MenuFooterRenderCallbacks& callbacks) {
    if (callbacks.copy_source_word_block) {
        callbacks.copy_source_word_block();
    }
    state.source_block_dirty = true;
    if (callbacks.refresh_menu_metadata) {
        callbacks.refresh_menu_metadata();
    }
    if (callbacks.draw_footer) {
        callbacks.draw_footer();
    }
}

} // namespace shadowrun::ghidra_conversion
