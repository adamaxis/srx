#include "shadowrun_ghidra/menus/menu_service_sequence_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    CyberwareMenuState cyberware{static_cast<std::uint16_t>(
                                    (1U << 0U) | (1U << 2U) | (1U << 14U) |
                                    (1U << 15U)),
                                0U};
    std::vector<int> cyberware_order;
    if (run_cyberware_menu(
            cyberware,
            {[&] { cyberware_order.push_back(1); },
             [&] { cyberware_order.push_back(2); },
             [&] { cyberware_order.push_back(3); },
             [&] { cyberware_order.push_back(4); },
             [&] { cyberware_order.push_back(5); }}) != 3U ||
        cyberware.rendered_entries != 3U ||
        cyberware_order != std::vector<int>{1, 2, 3, 3, 3, 3, 4, 5}) {
        return 100;
    }

    CyberwareMenuState empty_cyberware{};
    int empty_titles = 0;
    if (run_cyberware_menu(
            empty_cyberware,
            {{}, {}, [&] { ++empty_titles; }, {}, {}}) != 0U ||
        empty_titles != 1) {
        return 101;
    }

    std::vector<int> order;
    draw_menu_hud_panel_variant_a([&order] { order.push_back(1); });
    ActiveRunnerHudPanelPlan panel_plan;
    draw_active_runner_hud_panel(
        ActiveRunnerHudPanelInput{2, 0x1234U},
        [&](const ActiveRunnerHudPanelPlan& plan) { panel_plan = plan; order.push_back(2); });
    if (order != std::vector<int>{1, 2} || panel_plan.tile_origin != -0x7940 ||
        panel_plan.field_parameter != 0x1234U || panel_plan.rows != 4 ||
        panel_plan.words_per_row != 4) {
        return 1;
    }

    draw_active_runner_hud_panel(
        ActiveRunnerHudPanelInput{0, 0},
        [&](const ActiveRunnerHudPanelPlan& plan) { panel_plan = plan; });
    if (panel_plan.tile_origin != -0x796C) {
        return 23;
    }
    draw_active_runner_hud_panel(
        ActiveRunnerHudPanelInput{1, 0},
        [&](const ActiveRunnerHudPanelPlan& plan) { panel_plan = plan; });
    if (panel_plan.tile_origin != -0x7956) {
        return 24;
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

    panel_events.clear();
    prepare_menu_grid_variant_a(
        [&panel_events] { panel_events.push_back(26); });
    if (panel_events != std::vector<int>{26}) {
        return 17;
    }

    prepare_menu_grid_variant_b(
        [&panel_events] { panel_events.push_back(27); });
    if (panel_events != std::vector<int>{26, 27}) {
        return 18;
    }

    prepare_menu_grid_variant_c(
        [&panel_events] { panel_events.push_back(28); });
    if (panel_events != std::vector<int>{26, 27, 28}) {
        return 19;
    }

    write_menu_repeated_grid(
        [&panel_events] { panel_events.push_back(29); });
    if (panel_events != std::vector<int>{26, 27, 28, 29}) {
        return 20;
    }

    write_menu_repeated_grid_with_metadata(
        [&panel_events] { panel_events.push_back(30); });
    if (panel_events != std::vector<int>{26, 27, 28, 29, 30}) {
        return 21;
    }

    render_menu_text_asset(
        [&panel_events] { panel_events.push_back(31); });
    if (panel_events != std::vector<int>{26, 27, 28, 29, 30, 31}) {
        return 22;
    }

    if (select_hud_glyph_attribute_flags({0U, 0U, -1}) != 0x2000U ||
        select_hud_glyph_attribute_flags({5U, 1U, -1}) != 0x2000U ||
        select_hud_glyph_attribute_flags({6U, 1U, -1}) != 0x6000U ||
        select_hud_glyph_attribute_flags({14U, 2U, 0}) != 0x6000U ||
        select_hud_glyph_attribute_flags({15U, 2U, 0}) != 0x2000U) {
        return 25;
    }

    return 0;
}
