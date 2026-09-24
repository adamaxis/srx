#include "shadowrun_ghidra/menu_service_sequence_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<int> order;
    draw_menu_hud_panel_variant_a([&order] { order.push_back(1); });
    draw_menu_hud_panel_variant_b([&order] { order.push_back(2); });
    if (order != std::vector<int>{1, 2}) {
        return 1;
    }

    order.clear();
    if (render_menu_tile_rows([&order] { order.push_back(3); },
                              [&order] { order.push_back(4); }) != 0U ||
        order.size() != 16U || order[0] != 3 || order[1] != 4 ||
        order[14] != 3 || order[15] != 4) {
        return 2;
    }
    return 0;
}
