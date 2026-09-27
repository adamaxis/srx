#include "shadowrun_ghidra/field/field_action_selection_navigation_conversion.hpp"

#include <string>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldActionSelectionNavigationState normal{};
    normal.menu_result = 0;
    std::vector<std::string> order;
    FieldActionSelectionNavigationCallbacks callbacks{
        [&] { order.emplace_back("prepare"); },
        [&] { order.emplace_back("grid"); },
        [&] { order.emplace_back("text"); },
        [&] { order.emplace_back("sprites"); },
        [&] { order.emplace_back("reset"); },
    };

    const auto negative_grid = plan_field_action_selection_panel_grid(-1);
    const auto zero_grid = plan_field_action_selection_panel_grid(0);
    const auto suppressed_grid = plan_field_action_selection_panel_grid(1);
    const auto positive_grid = plan_field_action_selection_panel_grid(2);
    const auto negative_selection =
        plan_field_action_selection_render(-1, 4);
    const auto zero_selection = plan_field_action_selection_render(0, 3);
    const auto sprite_plan = plan_field_action_selection_sprites(-1);
    if (!expect(negative_grid.variant ==
                    FieldActionSelectionPanelGridVariant::detail_result &&
                negative_grid.writes_grid &&
                negative_grid.destination_word == -0x17F0 &&
                negative_grid.grid_write_count == 0x12U &&
                zero_grid.variant == FieldActionSelectionPanelGridVariant::ordinary_action_result &&
                zero_grid.destination_word == -0x17EA &&
                zero_grid.grid_write_count == 0x0CU &&
                suppressed_grid.variant ==
                    FieldActionSelectionPanelGridVariant::equipment_upgrade_result &&
                !suppressed_grid.writes_grid &&
                positive_grid.variant ==
                    FieldActionSelectionPanelGridVariant::resource_purchase_result &&
                negative_selection.family ==
                    FieldActionSelectionGridFamily::detail_result &&
                negative_selection.grid_record_offset == 0x20U &&
                negative_selection.text_record_offset == 0xA8U &&
                negative_selection.screen_column_offset == 0x18U &&
                zero_selection.family ==
                    FieldActionSelectionGridFamily::ordinary_action_result &&
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

    FieldActionSelectionNavigationState sprite_state{};
    sprite_state.action_cursor = 2;
    std::vector<std::string> sprite_order;
    FieldActionSelectionNavigationCallbacks sprite_callbacks{};
    sprite_callbacks.build_field_action_cursor_sprites =
        [&] { sprite_order.emplace_back("sprites"); };
    sprite_callbacks.render_field_action_selection =
        [&] { sprite_order.emplace_back("action_selection"); };
    refresh_field_action_selection_sprites(sprite_state, sprite_callbacks);
    if (!expect(sprite_order ==
                    std::vector<std::string>{"sprites", "action_selection"} &&
                sprite_state.sprite_records_dirty &&
                sprite_state.sprite_list_dirty &&
                sprite_state.display_dirty_flags == 2U)) {
        return 11;
    }
    advance_field_action_selection_cursor(normal, 1, callbacks);
    if (!expect(normal.action_cursor == 1 && normal.action_panel == 0 &&
                order == std::vector<std::string>{"prepare", "grid", "grid", "text"})) {
        return 1;
    }

    advance_field_action_selection_cursor(normal, 9, callbacks);
    if (!expect(normal.action_cursor == 3 &&
                order == std::vector<std::string>{"prepare", "grid", "grid", "text"})) {
        return 2;
    }
    advance_field_action_selection_cursor(normal, -9, callbacks);
    if (!expect(normal.action_cursor == 0)) {
        return 3;
    }

    normal.menu_result = 2;
    advance_field_action_selection_cursor(normal, 9, callbacks);
    if (!expect(normal.action_cursor == 5 && order.back() == "text")) {
        return 4;
    }
    advance_field_action_selection_cursor(normal, -4, callbacks);
    if (!expect(normal.action_cursor == 1 && order.back() == "text")) {
        return 41;
    }

    advance_field_action_selection_panel(normal, 1, callbacks);
    if (!expect(normal.action_panel == 1 && normal.action_cursor == 0 &&
                order.size() >= 3U && order[order.size() - 3] == "prepare" &&
                order[order.size() - 2] == "grid" &&
                order.back() == "sprites")) {
        return 5;
    }
    advance_field_action_selection_cursor(normal, 9, callbacks);
    if (!expect(normal.action_cursor == 7 && normal.sprite_records_dirty &&
                normal.sprite_list_dirty && normal.display_dirty_flags == 2U &&
                order.back() == "sprites")) {
        return 6;
    }
    advance_field_action_selection_cursor(normal, 6, callbacks);
    if (!expect(normal.action_cursor == 7 && order.back() == "sprites")) {
        return 61;
    }
    advance_field_action_selection_panel(normal, -1, callbacks);
    if (!expect(normal.action_panel == 0 && normal.action_cursor == 0 &&
                order[order.size() - 5] == "reset" &&
                order[order.size() - 4] == "prepare" &&
                order[order.size() - 3] == "grid" &&
                order[order.size() - 2] == "grid" &&
                order.back() == "text")) {
        return 7;
    }

    FieldActionSelectionNavigationState result_one{};
    result_one.menu_result = 1;
    std::vector<std::string> result_one_order;
    const FieldActionSelectionNavigationCallbacks result_one_callbacks{
        [&] { result_one_order.emplace_back("prepare"); },
        [&] { result_one_order.emplace_back("grid"); },
        [&] { result_one_order.emplace_back("text"); },
        [&] { result_one_order.emplace_back("sprites"); },
        [&] { result_one_order.emplace_back("reset"); },
    };
    advance_field_action_selection_cursor(result_one, 1, result_one_callbacks);
    if (!expect(result_one.action_cursor == 1 &&
                result_one_order == std::vector<std::string>{"prepare"})) {
        return 8;
    }
    advance_field_action_selection_panel(result_one, 1, result_one_callbacks);
    if (!expect(result_one_order ==
                std::vector<std::string>{"prepare", "prepare", "sprites"})) {
        return 9;
    }

    return 0;
}
