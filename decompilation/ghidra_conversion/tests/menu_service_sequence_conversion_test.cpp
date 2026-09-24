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

    order.clear();
    reset_and_upload_menu_tile_block([&order] { order.push_back(5); },
                                     [&order] { order.push_back(6); });
    if (order != std::vector<int>{5, 6}) {
        return 3;
    }

    write_menu_grid([&order] { order.push_back(7); });
    if (order != std::vector<int>{5, 6, 7}) {
        return 4;
    }

    order.clear();
    write_menu_grid_quad([&order] { order.push_back(8); },
                         [&order] { order.push_back(9); });
    if (order != std::vector<int>{8, 8, 8, 8, 9}) {
        return 5;
    }
    return 0;
}
