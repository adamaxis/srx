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
    return 0;
}
