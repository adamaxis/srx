#include "shadowrun_ghidra/menu_transfer_batch_conversion.hpp"

#include <cstdint>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    std::vector<int> events;
    write_repeated_menu_grid([&] { events.push_back(1); });
    prepare_menu_grid_then_service([&] { events.push_back(2); },
                                   [&] { events.push_back(3); });
    prepare_grid_then_write([&] { events.push_back(4); },
                            [&] { events.push_back(5); });
    write_three_grids_then_service([&] { events.push_back(6); },
                                   [&] { events.push_back(7); });
    if (events != std::vector<int>{1, 2, 3, 4, 5, 6, 6, 6, 7}) {
        return 1;
    }

    std::vector<std::int16_t> scanned;
    if (!find_matching_menu_metric([&](const std::int16_t slot) {
            scanned.push_back(slot);
            return slot == 3;
        }) ||
        scanned != std::vector<std::int16_t>{6, 5, 4, 3}) {
        return 2;
    }

    events.clear();
    upload_resource_then_write_two_blocks([&] { events.push_back(1); },
                                          [&] { events.push_back(2); });
    if (events != std::vector<int>{1, 2, 2}) {
        return 3;
    }
    upload_resource_stream([&] { events.push_back(3); });
    draw_hud_panel_wrapper([&] { events.push_back(4); });
    render_title_text_source_wrapper([&] { events.push_back(5); });
    render_title_text_source_leaf([&] { events.push_back(6); });
    render_title_text_source_variant([&] { events.push_back(7); });
    if (events != std::vector<int>{1, 2, 2, 3, 4, 5, 6, 7}) {
        return 4;
    }

    EntityAuxiliaryState auxiliary{9};
    clear_entity_auxiliary_marker(auxiliary);
    if (auxiliary.marker != 0) {
        return 5;
    }
    write_single_grid([&] { events.push_back(8); });
    if (events.back() != 8) {
        return 6;
    }

    ResourceVariantState variant{};
    int uploads = 0;
    if (!update_resource_variant(variant, 13, [&] { ++uploads; }) ||
        !variant.alternate || uploads != 1 ||
        update_resource_variant(variant, 14, [&] { ++uploads; }) ||
        uploads != 1) {
        return 7;
    }
    if (passes_entity_action_threshold(1, -1, 10, {})) {
        // Phase one is the intentional bypass.
    } else {
        return 8;
    }
    if (passes_entity_action_threshold(2, 5, 10, {})) {
        return 9;
    }
    if (!passes_entity_action_threshold(2, 10, 10, {})) {
        return 10;
    }

    PaletteFadeState fade{true, 2, false, false};
    int effects = 0;
    int palette_steps = 0;
    advance_palette_fade(fade, {[&] { ++effects; }, [&] { ++palette_steps; }});
    advance_palette_fade(fade, {[&] { ++effects; }, [&] { ++palette_steps; }});
    advance_palette_fade(fade, {[&] { ++effects; }, [&] { ++palette_steps; }});
    if (effects != 3 || palette_steps != 2 || !fade.dirty || fade.active) {
        return 11;
    }

    CoordinateProjectionInput coordinate{0x0100, 0x0400, 0};
    if (project_entity_x_axis(coordinate, 20) != 22 ||
        project_entity_y_axis(coordinate, 20) != 0) {
        return 12;
    }

    int forwards = 0;
    draw_hud_panel_forward([&] { ++forwards; });
    draw_hud_footer_forward([&] { ++forwards; });
    if (forwards != 2) {
        return 13;
    }
    return 0;
}
