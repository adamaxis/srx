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

void upload_menu_tile_block(
    MenuTileUploadState& state,
    const MenuTileUploadBoundary& upload_tile_block) {
    if (upload_tile_block) {
        upload_tile_block();
    }
    ++state.uploaded_blocks;
}

void reset_menu_upload_cursor_and_upload(
    MenuTileUploadState& state,
    const MenuTileUploadBoundary& upload_tile_block) {
    state.upload_cursor = 0U;
    upload_menu_tile_block(state, upload_tile_block);
}

void reset_menu_overlay_and_upload_two(
    MenuTileUploadState& state,
    const MenuTileUploadBoundary& upload_tile_block) {
    state.overlay_state = 0U;
    for (std::size_t block = 0; block < 2U; ++block) {
        upload_menu_tile_block(state, upload_tile_block);
    }
}

} // namespace shadowrun::ghidra_conversion
