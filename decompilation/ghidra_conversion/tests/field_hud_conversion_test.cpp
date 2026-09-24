#include "shadowrun_ghidra/field_hud_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<int> order;
    std::uint32_t uploads = 0U;
    const auto callbacks = FieldHudRenderCallbacks{
        [&order] { order.push_back(1); },
        [&order] { order.push_back(2); },
        [&order, &uploads] {
            order.push_back(3);
            ++uploads;
        },
        [&order] { order.push_back(4); },
        [&order] {
            order.push_back(5);
            return static_cast<std::uint8_t>(8U);
        },
        [&order] { order.push_back(6); },
        [&order] { order.push_back(7); },
        [&order] { order.push_back(8); },
    };
    const auto rendered = render_field_hud(true, callbacks);
    if (!expect(rendered.uploaded_tile_blocks == 5U && uploads == 5U &&
                rendered.mode_panel_rendered &&
                order == std::vector<int>{1, 2, 3, 3, 3, 3, 3, 4, 5, 6,
                                           7, 8})) {
        return 1;
    }

    order.clear();
    const auto skipped = render_field_hud(
        false,
        {
            [&order] { order.push_back(1); },
            [&order] { order.push_back(2); },
            [&order] { order.push_back(3); },
            [&order] { order.push_back(4); },
            [&order] {
                order.push_back(5);
                return static_cast<std::uint8_t>(8U);
            },
            [&order] { order.push_back(6); },
            [&order] { order.push_back(7); },
            [&order] { order.push_back(8); },
        });
    if (!expect(skipped.uploaded_tile_blocks == 5U &&
                !skipped.mode_panel_rendered &&
                order == std::vector<int>{1, 2, 3, 3, 3, 3, 3, 4})) {
        return 2;
    }

    int panel_draws = 0;
    draw_hud_panel_triplet([&panel_draws] { ++panel_draws; });
    if (!expect(panel_draws == 3)) {
        return 3;
    }

    return 0;
}
