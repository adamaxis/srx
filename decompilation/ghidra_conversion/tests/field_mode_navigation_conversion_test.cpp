#include "shadowrun_ghidra/field_mode_navigation_conversion.hpp"

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
    advance_field_mode_primary_cursor(normal, 1, callbacks);
    if (!expect(normal.primary_cursor == 1 && normal.panel_cursor == 0 &&
                order == std::vector<std::string>{"prepare", "grid", "grid", "text"})) {
        return 1;
    }

    advance_field_mode_primary_cursor(normal, 9, callbacks);
    if (!expect(normal.primary_cursor == 3 &&
                order == std::vector<std::string>{"prepare", "grid", "grid", "text",
                                                   "prepare", "grid", "grid", "text"})) {
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
