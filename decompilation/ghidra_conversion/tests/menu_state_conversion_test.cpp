#include "shadowrun_ghidra/menu_state_conversion.hpp"

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
    return 0;
}
