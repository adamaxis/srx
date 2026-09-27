#include "shadowrun_ghidra/field/field_mode_navigation_conversion.hpp"

#include <string>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldModeNavigationState normal{};
    normal.mode_result = 0;
    std::vector<std::string> order;
    FieldModeNavigationCallbacks callbacks{
        [&] { order.emplace_back("prepare"); },
        [&] { order.emplace_back("grid"); },
        [&] { order.emplace_back("text"); },
        [&] { order.emplace_back("sprites"); },
        [&] { order.emplace_back("reset"); },
    };

    const auto negative_grid = plan_field_mode_panel_grid(-1);
    const auto zero_grid = plan_field_mode_panel_grid(0);
    const auto suppressed_grid = plan_field_mode_panel_grid(1);
    const auto positive_grid = plan_field_mode_panel_grid(2);
    const auto negative_selection =
        plan_field_mode_selection_render(-1, 4);
    const auto zero_selection = plan_field_mode_selection_render(0, 3);
    const auto sprite_plan = plan_field_mode_selection_sprites(-1);
    if (!expect(negative_grid.variant ==
                    FieldModePanelGridVariant::negative_result &&
                negative_grid.writes_grid &&
                negative_grid.destination_word == -0x17F0 &&
                negative_grid.grid_write_count == 0x12U &&
                zero_grid.variant == FieldModePanelGridVariant::zero_result &&
                zero_grid.destination_word == -0x17EA &&
                zero_grid.grid_write_count == 0x0CU &&
                suppressed_grid.variant ==
                    FieldModePanelGridVariant::result_one_suppressed &&
                !suppressed_grid.writes_grid &&
                positive_grid.variant ==
                    FieldModePanelGridVariant::positive_result &&
                negative_selection.family ==
                    FieldModeSelectionGridFamily::negative_result &&
                negative_selection.grid_record_offset == 0x20U &&
                negative_selection.text_record_offset == 0xA8U &&
                negative_selection.screen_column_offset == 0x18U &&
                zero_selection.family ==
                    FieldModeSelectionGridFamily::zero_result &&
                zero_selection.grid_record_offset == 0x18U &&
                zero_selection.text_record_offset == 0x7EU &&
                zero_selection.screen_column_offset == 0x12U &&
                sprite_plan.source_record_offset == 0x1FE0U &&
                sprite_plan.sprite_build_counter == 2U &&
                sprite_plan.sprite_palette_attribute == 0x4000U &&
                sprite_plan.clears_trailing_marker &&
                sprite_plan.renders_action_selection_after_build)) {
        return 10;
    }

    FieldModeNavigationState sprite_state{};
    sprite_state.primary_cursor = 2;
    std::vector<std::string> sprite_order;
    FieldModeNavigationCallbacks sprite_callbacks{};
    sprite_callbacks.build_selection_sprites =
        [&] { sprite_order.emplace_back("sprites"); };
    sprite_callbacks.render_action_selection =
        [&] { sprite_order.emplace_back("action_selection"); };
    refresh_field_mode_selection_sprites(sprite_state, sprite_callbacks);
    if (!expect(sprite_order ==
                    std::vector<std::string>{"sprites", "action_selection"} &&
                sprite_state.sprite_records_dirty &&
                sprite_state.sprite_list_dirty &&
                sprite_state.display_dirty_flags == 2U)) {
        return 11;
    }
    advance_field_mode_primary_cursor(normal, 1, callbacks);
    if (!expect(normal.primary_cursor == 1 && normal.panel_cursor == 0 &&
                order == std::vector<std::string>{"prepare", "grid", "grid", "text"})) {
        return 1;
    }

    advance_field_mode_primary_cursor(normal, 9, callbacks);
    if (!expect(normal.primary_cursor == 3 &&
                order == std::vector<std::string>{"prepare", "grid", "grid", "text"})) {
        return 2;
    }
    advance_field_mode_primary_cursor(normal, -9, callbacks);
    if (!expect(normal.primary_cursor == 0)) {
        return 3;
    }

    normal.mode_result = 2;
    advance_field_mode_primary_cursor(normal, 9, callbacks);
    if (!expect(normal.primary_cursor == 5 && order.back() == "text")) {
        return 4;
    }
    advance_field_mode_primary_cursor(normal, -4, callbacks);
    if (!expect(normal.primary_cursor == 1 && order.back() == "text")) {
        return 41;
    }

    advance_field_mode_panel_cursor(normal, 1, callbacks);
    if (!expect(normal.panel_cursor == 1 && normal.primary_cursor == 0 &&
                order.size() >= 3U && order[order.size() - 3] == "prepare" &&
                order[order.size() - 2] == "grid" &&
                order.back() == "sprites")) {
        return 5;
    }
    advance_field_mode_primary_cursor(normal, 9, callbacks);
    if (!expect(normal.primary_cursor == 7 && normal.sprite_records_dirty &&
                normal.sprite_list_dirty && normal.display_dirty_flags == 2U &&
                order.back() == "sprites")) {
        return 6;
    }
    advance_field_mode_primary_cursor(normal, 6, callbacks);
    if (!expect(normal.primary_cursor == 7 && order.back() == "sprites")) {
        return 61;
    }
    advance_field_mode_panel_cursor(normal, -1, callbacks);
    if (!expect(normal.panel_cursor == 0 && normal.primary_cursor == 0 &&
                order[order.size() - 5] == "reset" &&
                order[order.size() - 4] == "prepare" &&
                order[order.size() - 3] == "grid" &&
                order[order.size() - 2] == "grid" &&
                order.back() == "text")) {
        return 7;
    }

    FieldModeNavigationState result_one{};
    result_one.mode_result = 1;
    std::vector<std::string> result_one_order;
    const FieldModeNavigationCallbacks result_one_callbacks{
        [&] { result_one_order.emplace_back("prepare"); },
        [&] { result_one_order.emplace_back("grid"); },
        [&] { result_one_order.emplace_back("text"); },
        [&] { result_one_order.emplace_back("sprites"); },
        [&] { result_one_order.emplace_back("reset"); },
    };
    advance_field_mode_primary_cursor(result_one, 1, result_one_callbacks);
    if (!expect(result_one.primary_cursor == 1 &&
                result_one_order == std::vector<std::string>{"prepare"})) {
        return 8;
    }
    advance_field_mode_panel_cursor(result_one, 1, result_one_callbacks);
    if (!expect(result_one_order ==
                std::vector<std::string>{"prepare", "prepare", "sprites"})) {
        return 9;
    }

    return 0;
}
