#include "shadowrun_ghidra/menus/menu_state_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::array<MenuSlotState, 5> slots{};
    for (auto& slot : slots) {
        slot.status = 0xFFU;
        slot.timer = 0xFFU;
    }
    initialize_menu_slot_states(slots);
    for (const auto& slot : slots) {
        if (slot.status != 6U || slot.timer != 1U) {
            return 1;
        }
    }

    MenuFooterRenderState state{};
    std::vector<int> order;
    render_menu_footer(
        state,
        {
            [&order] { order.push_back(1); },
            [&order] { order.push_back(2); },
            [&order] { order.push_back(3); },
        });
    if (!state.source_block_dirty || order != std::vector<int>{1, 2, 3}) {
        return 2;
    }

    MenuTileUploadState upload_state{7U, 9U, 0U};
    std::size_t upload_calls = 0;
    upload_menu_tile_block(upload_state, [&upload_calls] { ++upload_calls; });
    if (upload_state.uploaded_blocks != 1U || upload_calls != 1U) {
        return 3;
    }
    reset_menu_upload_cursor_and_upload(
        upload_state, [&upload_calls] { ++upload_calls; });
    if (upload_state.upload_cursor != 0U ||
        upload_state.uploaded_blocks != 2U || upload_calls != 2U) {
        return 4;
    }
    reset_menu_overlay_and_upload_two(
        upload_state, [&upload_calls] { ++upload_calls; });
    if (upload_state.overlay_state != 0U ||
        upload_state.uploaded_blocks != 4U || upload_calls != 4U) {
        return 5;
    }

    const auto block = plan_menu_tile_block(0x000CD3B6U);
    if (block.glyph_source != 0x000CD3B6U ||
        block.destination != kMenuTileBlockDestination ||
        block.column_max != kMenuTileBlockColumnMax ||
        block.row_max != kMenuTileBlockRowMax) {
        return 6;
    }
    MenuColumnTileUploadState column{};
    column.column = 2U;
    column.layout_word = 1;
    column.glyph_base = 0x1000U;
    reset_menu_upload_cursor_and_upload(column);
    if (column.layout_word != 0 || column.grid.glyph_source != 0x1000U ||
        column.grid.destination != kMenuColumnGridDestinationNonneg ||
        column.grid.column_max != 1U || column.grid.row_max != 1U) {
        return 7;
    }
    column.column = 3U;
    column.layout_word = -1;
    column.glyph_base = 0x1000U;
    reset_menu_upload_cursor_and_upload(column);
    if (column.layout_word != 0 || column.grid.glyph_source != 0x1008U ||
        column.grid.destination != kMenuColumnGridDestinationNeg + 6U) {
        return 8;
    }
    return 0;
}
