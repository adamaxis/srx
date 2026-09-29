#include "shadowrun_ghidra/menus/menu_state_conversion.hpp"

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

MenuTileGridPlan plan_menu_tile_block(const std::uint32_t glyph_source) {
    MenuTileGridPlan plan{};
    plan.glyph_source = glyph_source;
    plan.destination = kMenuTileBlockDestination;
    plan.column_max = kMenuTileBlockColumnMax;
    plan.row_max = kMenuTileBlockRowMax;
    return plan;
}

void upload_menu_tile_block(
    MenuTileUploadState& state,
    const MenuTileUploadBoundary& upload_tile_block) {
    if (upload_tile_block) {
        upload_tile_block();
    }
    ++state.uploaded_blocks;
}

void reset_menu_upload_cursor_and_upload(MenuColumnTileUploadState& state) {
    const auto slot = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(state.column) - 2U);
    const auto source_delta = static_cast<std::uint32_t>(slot) * 8U;
    const auto dest_delta = static_cast<std::uint32_t>(slot) * 6U;
    state.grid.glyph_source = state.glyph_base + source_delta;
    state.grid.destination =
        dest_delta + (state.layout_word < 0 ? kMenuColumnGridDestinationNeg
                                            : kMenuColumnGridDestinationNonneg);
    state.grid.column_max = 1;
    state.grid.row_max = 1;
    state.grid.palette = 0;
    state.layout_word = 0;
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
