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

    order.clear();
    prepare_menu_row_grid([&order] { order.push_back(10); },
                          [&order] { order.push_back(11); });
    if (order != std::vector<int>{10, 11}) {
        return 6;
    }

    order.clear();
    render_menu_row([&order] { order.push_back(12); },
                    [&order] { order.push_back(13); });
    if (order != std::vector<int>{12, 13}) {
        return 7;
    }

    order.clear();
    if (prepare_menu_dialog(
            [&order] {
                order.push_back(14);
                return std::uint64_t{0x12345678U};
            },
            [&order] { order.push_back(15); }) != 0x12345678U ||
        order != std::vector<int>{14, 15, 15}) {
        return 8;
    }

    MenuTextRefreshState text_state{-1};
    order.clear();
    if (!refresh_menu_text(text_state,
                           [&order] { order.push_back(16); },
                           [&order] { order.push_back(17); }) ||
        text_state.pending_text_count != 0 ||
        order != std::vector<int>{16, 17, 17}) {
        return 9;
    }
    text_state.pending_text_count = 1;
    if (refresh_menu_text(text_state, [&order] { order.push_back(18); },
                         [&order] { order.push_back(19); })) {
        return 10;
    }

    MenuTextCursorState cursor{0, -1, 3};
    order.clear();
    advance_menu_text_cursor(cursor, [&order] { order.push_back(20); },
                             [&order] { order.push_back(21); });
    if (cursor.selector != 0 || order != std::vector<int>{20, 21}) {
        return 11;
    }

    MenuNumberRenderState number{2, 5};
    std::vector<std::int16_t> values;
    std::uint16_t marker = 0;
    render_menu_number(number, 1,
                       {[&marker](const std::uint16_t word) { marker = word; },
                        [&values](const std::int16_t value) {
                            values.push_back(value);
                        }});
    if (marker != 0x6000U || number.cursor != 3 ||
        values != std::vector<std::int16_t>{3}) {
        return 12;
    }

    render_menu_number(number, 3, {{}, [&values](const std::int16_t value) {
                                       values.push_back(value);
                                   }});
    if (number.cursor != 1 || values != std::vector<std::int16_t>{3, 1}) {
        return 13;
    }

    MenuSlotCleanupState slot;
    slot.pending_actions[3] = 8;
    slot.active_slots[3] = 1;
    std::vector<int> slot_events;
    clear_menu_slot(slot, 10, 3, true,
                    [&slot_events] { slot_events.push_back(22); });
    if (slot.slot_type != 0x7FU || slot.slot_subtype != 0U ||
        slot.pending_actions[3] != 0U || slot.active_slots[3] != 0U ||
        slot_events != std::vector<int>{22}) {
        return 14;
    }

    slot.action_marker = 5;
    slot.pending_actions[2] = 1;
    slot.active_slots[2] = 1;
    clear_menu_slot(slot, 0x30, 2, false, {});
    if (slot.action_marker != 0 || slot.pending_actions[2] != 0U ||
        slot.active_slots[2] != 0U) {
        return 15;
    }

    MenuPanelRefreshState panel{0x12U};
    std::vector<int> panel_events;
    refresh_menu_panel(panel,
                       [&panel_events] { panel_events.push_back(23); },
                       [&panel_events] { panel_events.push_back(24); },
                       [&panel_events] { panel_events.push_back(25); });
    if (panel.toggle != 0xEDU ||
        panel_events != std::vector<int>{23, 24, 25}) {
        return 16;
    }

    return 0;
}
