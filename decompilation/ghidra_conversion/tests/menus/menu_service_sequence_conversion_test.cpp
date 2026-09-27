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
    CommandOverviewHudPanelPlan overview_panel;
    draw_command_overview_hud_panel(
        CommandOverviewHudPanelInput{2U, 0x1234U},
        [&](const CommandOverviewHudPanelPlan& plan) {
            overview_panel = plan;
            order.push_back(1);
        });
    if (order != std::vector<int>{1} ||
        overview_panel.tile_origin != static_cast<std::int16_t>(0x80A1U) ||
        overview_panel.destination_tile_base != 0xE104U ||
        overview_panel.field_parameter != 0x1234U || overview_panel.rows != 10U ||
        overview_panel.words_per_row != 8U) {
        return 1;
    }

    ActiveRunnerHudPanelPlan panel_plan;
    draw_active_runner_hud_panel(
        ActiveRunnerHudPanelInput{2U, 0x1234U, 0xC928U},
        [&](const ActiveRunnerHudPanelPlan& plan) { panel_plan = plan; order.push_back(2); });
    if (order != std::vector<int>{1, 2} || panel_plan.tile_origin != -0x7940 ||
        panel_plan.field_parameter != 0x1234U ||
        panel_plan.destination_tile_base != 0xC928U || panel_plan.rows != 4 ||
        panel_plan.words_per_row != 4) {
        return 2;
    }

    draw_active_runner_hud_panel(
        ActiveRunnerHudPanelInput{0U, 0U, 0xE132U},
        [&](const ActiveRunnerHudPanelPlan& plan) { panel_plan = plan; });
    if (panel_plan.tile_origin != -0x796C) {
        return 23;
    }
    draw_active_runner_hud_panel(
        ActiveRunnerHudPanelInput{1U, 0U, 0x1CBAU},
        [&](const ActiveRunnerHudPanelPlan& plan) { panel_plan = plan; });
    if (panel_plan.tile_origin != -0x7956 ||
        panel_plan.destination_tile_base != 0x1CBAU) {
        return 24;
    }

    ActiveRunnerNameTextPlan name_plan;
    render_active_runner_name_text(
        [&name_plan](const ActiveRunnerNameTextPlan& plan) { name_plan = plan; });
    if (name_plan.source_record_offset != 0x58U ||
        name_plan.destination_tile_base != 0xC0A0U ||
        name_plan.palette_flags != 0x6000U) {
        return 25;
    }

    const auto inventory_title = select_command_overview_title_text(0U);
    const auto clips_title = select_command_overview_title_text(2U);
    const auto magic_title = select_menu_title_text(
        MenuTitleTextInput{5U, MenuTextTableRef{"menu.magic.labels"}});
    if (!inventory_title.has_value() ||
        inventory_title->source_alias != "menu.inventory.labels" ||
        inventory_title->record_index != 0U ||
        inventory_title->record_offset_bytes != 0U ||
        !inventory_title->used_inventory_fallback || !clips_title.has_value() ||
        clips_title->source_alias != "menu.command.labels" ||
        clips_title->record_index != 0U ||
        clips_title->record_offset_bytes != 0U ||
        clips_title->used_inventory_fallback || !magic_title.has_value() ||
        magic_title->source_alias != "menu.magic.labels" ||
        magic_title->record_index != 3U ||
        magic_title->record_offset_bytes != 0x36U ||
        select_menu_title_text(MenuTitleTextInput{2U, {}}).has_value()) {
        return 26;
    }

    order.clear();
    if (render_menu_tile_rows([&order] { order.push_back(3); },
                              [&order] { order.push_back(4); }) != 0U ||
        order.size() != 16U || order[0] != 3 || order[1] != 4 ||
        order[14] != 3 || order[15] != 4) {
        return 2;
    }

    order.clear();
    if (reset_and_upload_menu_tile_block(
            [&order] { order.push_back(5); },
            [&order] { order.push_back(6); }, -1) != -1 ||
        order != std::vector<int>{5, 6}) {
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
        select_hud_glyph_attribute_flags({15U, 2U, 0}) != 0x2000U ||
        select_hud_glyph_attribute_flags({-1, 0, -1}) != 0x2000U ||
        select_hud_glyph_attribute_flags({-1, 2, -1}) != 0x6000U ||
        select_hud_glyph_attribute_flags({6, -1, -1}) != 0x6000U ||
        select_hud_glyph_attribute_flags(
            {static_cast<std::int8_t>(0x80),
             static_cast<std::int8_t>(0x80), -1}) != 0x2000U ||
        select_hud_glyph_attribute_flags(
            {static_cast<std::int8_t>(0x80), 1, -1}) != 0x6000U ||
        select_hud_glyph_attribute_flags(
            {static_cast<std::int8_t>(0xFF), 1, -1}) != 0x2000U) {
        return 25;
    }

    CommandOverviewActionRefreshState action_state{3U, 2U, 0U, false, 0x01U};
    std::vector<int> action_order;
    CommandOverviewSpriteBuildPlan sprite_plan;
    CommandOverviewActionRefreshResult action_result =
        refresh_command_overview_action_details(
            action_state,
            {[&](const CommandOverviewSpriteBuildPlan& plan) {
                 sprite_plan = plan;
                 action_order.push_back(32);
                 return std::size_t{4U};
             },
             [&] { action_order.push_back(33); },
             [&] { action_order.push_back(34); },
             [&] {
                 action_order.push_back(35);
                 return std::uint8_t{7U};
             },
             [&] {
                 action_order.push_back(36);
             },
             [&] { action_order.push_back(37); },
             [&] { action_order.push_back(38); },
             {}});
    if (action_order != std::vector<int>{32, 33, 34, 35, 36, 37, 38} ||
        sprite_plan.source_alias != "menu.field.cursor" ||
        sprite_plan.y_offset != 120 || sprite_plan.x_offset != 48 ||
        sprite_plan.first_sequence != 2U ||
        sprite_plan.attribute_flags != 0x2000U ||
        action_state.sprite_record_count != 5U ||
        !action_state.sprite_list_ready ||
        action_state.display_dirty_flags != 0x03U ||
        action_result.action_code != 7U ||
        action_result.item_details_result != 7U ||
        !action_result.item_details_rendered ||
        !action_result.action_menu_serviced || !action_result.dialog_prepared ||
        action_result.action_title_rendered) {
        return 32;
    }

    action_state = CommandOverviewActionRefreshState{0U, 1U, 99U, false, 0U};
    action_order.clear();
    CommandOverviewActionTitleTextPlan title_plan;
    action_result = refresh_command_overview_action_details(
        action_state,
        {{},
         {},
         [&] { action_order.push_back(39); },
         [&] {
             action_order.push_back(40);
             return std::uint8_t{0U};
         },
         {},
         {},
         {},
         [&](const CommandOverviewActionTitleTextPlan& plan) {
             title_plan = plan;
             action_order.push_back(41);
         }});
    if (action_order != std::vector<int>{39, 40, 41} ||
        action_state.sprite_record_count != 1U ||
        action_state.display_dirty_flags != 0x02U ||
        !action_result.action_title_rendered ||
        action_result.action_code != 0U ||
        title_plan.source_table !=
            "menu.field.command_overview_action_title_table" ||
        title_plan.destination_tile_base != 0xC520U ||
        title_plan.palette_flags != 0x6000U) {
        return 33;
    }

    CommandOverviewPrimaryPageState primary_page{1U, 0};
    std::vector<int> primary_order;
    advance_command_overview_primary_page(
        primary_page,
        {[&] {
             primary_order.push_back(42);
             return std::int8_t{1};
         },
         [&] { primary_order.push_back(43); },
         [&] { primary_order.push_back(44); },
         [&] { primary_order.push_back(45); }});
    if (primary_page.page != 2U || primary_page.auxiliary_marker != -1 ||
        primary_order != std::vector<int>{42, 43, 44}) {
        return 34;
    }

    primary_page = {2U, 0};
    primary_order.clear();
    advance_command_overview_primary_page(
        primary_page,
        {[&] {
             primary_order.push_back(46);
             return std::int8_t{-1};
         },
         [&] { primary_order.push_back(47); },
         [&] { primary_order.push_back(48); },
         [&] { primary_order.push_back(49); }});
    if (primary_page.page != 1U ||
        primary_order != std::vector<int>{46, 49}) {
        return 35;
    }

    primary_page = {6U, 0};
    primary_order.clear();
    advance_command_overview_primary_page(
        primary_page,
        {[&] {
             primary_order.push_back(50);
             return std::int8_t{1};
         },
         [&] { primary_order.push_back(51); },
         [&] { primary_order.push_back(52); },
         [&] { primary_order.push_back(53); }});
    if (primary_page.page != 0U ||
        primary_order != std::vector<int>{50, 53}) {
        return 36;
    }

    CommandOverviewSecondaryPageState secondary_page{2U, 0U, 0, 0U};
    std::vector<int> secondary_order;
    advance_command_overview_secondary_page(
        secondary_page,
        {[&] {
             secondary_order.push_back(54);
             return std::int8_t{0};
         },
         [&] { secondary_order.push_back(55); },
         [&] { secondary_order.push_back(56); },
         [&] { secondary_order.push_back(57); }});
    if (secondary_page.action_dialog_gate != 0 ||
        secondary_order != std::vector<int>{54, 55, 56}) {
        return 37;
    }

    secondary_page = {2U, 0U, -1, 0U};
    secondary_order.clear();
    advance_command_overview_secondary_page(
        secondary_page,
        {[&] {
             secondary_order.push_back(58);
             return std::int8_t{0};
         },
         [&] { secondary_order.push_back(59); },
         [&] { secondary_order.push_back(60); },
         [&] { secondary_order.push_back(61); }});
    if (secondary_page.action_dialog_gate != 0 || !secondary_order.empty()) {
        return 38;
    }

    secondary_page = {2U, 0U, 7, static_cast<std::uint16_t>(1U << 2U)};
    secondary_order.clear();
    advance_command_overview_secondary_page(
        secondary_page,
        {[&] {
             secondary_order.push_back(62);
             return std::int8_t{0};
         },
         [&] { secondary_order.push_back(63); },
         [&] { secondary_order.push_back(64); },
         [&] { secondary_order.push_back(65); }});
    if (secondary_page.action_dialog_gate != 7 || !secondary_order.empty()) {
        return 39;
    }

    secondary_page = {1U, 3U, 0, 0U};
    secondary_order.clear();
    advance_command_overview_secondary_page(
        secondary_page,
        {[&] {
             secondary_order.push_back(66);
             return std::int8_t{1};
         },
         [&] { secondary_order.push_back(67); },
         [&] { secondary_order.push_back(68); },
         [&] { secondary_order.push_back(69); }});
    if (secondary_page.secondary_page != 3U ||
        secondary_order != std::vector<int>{66, 69}) {
        return 40;
    }

    secondary_page = {1U, 1U, 0, 0U};
    secondary_order.clear();
    advance_command_overview_secondary_page(
        secondary_page,
        {[&] {
             secondary_order.push_back(70);
             return std::int8_t{-2};
         },
         {},
         {},
         [&] { secondary_order.push_back(71); }});
    if (secondary_page.secondary_page != 0U ||
        secondary_order != std::vector<int>{70, 71}) {
        return 41;
    }

    std::vector<std::uint16_t> dialog_events{0x0800U, 0x0400U, 0x8000U};
    std::size_t dialog_event_index = 0U;
    std::vector<std::int8_t> dialog_preparations;
    std::vector<std::uint16_t> dialog_audio;
    std::vector<CommandOverviewActionDialogGridPlan> dialog_grids;
    std::vector<CommandOverviewActionDialogBarPlan> dialog_bars;
    int dialog_vblanks = 0;
    int dialog_latches = 0;
    int posture_adjustments = 0;
    const auto dialog_result = run_command_overview_action_dialog(
        CommandOverviewActionDialogInput{},
        {[&] { ++dialog_vblanks; },
         [&] { ++dialog_latches; },
         [&] { return dialog_events.at(dialog_event_index++); },
         [&](const std::uint16_t event) { dialog_audio.push_back(event); },
         [&](const CommandOverviewActionDialogPreparationPlan& plan) {
             dialog_preparations.push_back(plan.posture_delta);
         },
         [&](const CommandOverviewActionDialogGridPlan& plan) {
             dialog_grids.push_back(plan);
         },
         [&] { ++posture_adjustments; },
         [&](const CommandOverviewActionDialogBarPlan& plan) {
             dialog_bars.push_back(plan);
         }});
    if (!dialog_result.initial_grid_written ||
        !dialog_result.final_grid_written ||
        !dialog_result.closed_by_controller ||
        !dialog_result.exit_marker_set ||
        dialog_result.closed_by_armed_gate ||
        dialog_result.controller_input_missing || dialog_result.iterations != 3U ||
        dialog_result.posture_deltas != std::vector<std::int8_t>{1, -1} ||
        dialog_preparations != std::vector<std::int8_t>{0, 1, -1} ||
        dialog_audio != std::vector<std::uint16_t>{0x25U, 0x25U} ||
        dialog_vblanks != 3 || dialog_latches != 3 || posture_adjustments != 1 ||
        dialog_grids.size() != 2U ||
        dialog_grids[0].kind != CommandOverviewActionDialogGridKind::initial ||
        dialog_grids[1].kind != CommandOverviewActionDialogGridKind::final ||
        dialog_grids[0].destination_tile_base != 0xCA9EU ||
        dialog_grids[0].outer_count != 0U || dialog_grids[0].inner_count != 5U ||
        dialog_grids[0].palette_flags != 0U ||
        dialog_bars.size() != 2U || dialog_bars[0].mode != 0 ||
        dialog_bars[0].destination_tile_base != 0xC69CU ||
        dialog_bars[1].mode != -1 ||
        dialog_bars[1].destination_tile_base != 0xC79CU) {
        return 42;
    }

    dialog_events = {0x0608U, 0x8000U};
    dialog_event_index = 0U;
    dialog_preparations.clear();
    dialog_audio.clear();
    const auto bit_priority_result = run_command_overview_action_dialog(
        CommandOverviewActionDialogInput{},
        {{},
         {},
         [&] { return dialog_events.at(dialog_event_index++); },
         [&](const std::uint16_t event) { dialog_audio.push_back(event); },
         [&](const CommandOverviewActionDialogPreparationPlan& plan) {
             dialog_preparations.push_back(plan.posture_delta);
         },
         {},
         {},
         {}});
    if (bit_priority_result.posture_deltas != std::vector<std::int8_t>{-1} ||
        dialog_preparations != std::vector<std::int8_t>{0, -1} ||
        dialog_audio != std::vector<std::uint16_t>{0x25U} ||
        !bit_priority_result.closed_by_controller) {
        return 43;
    }

    std::vector<std::uint16_t> armed_gate_events{0x0200U};
    std::size_t armed_gate_index = 0U;
    int armed_gate_adjustments = 0;
    const auto armed_gate_result = run_command_overview_action_dialog(
        CommandOverviewActionDialogInput{-1},
        {{},
         {},
         [&] { return armed_gate_events.at(armed_gate_index++); },
         {},
         {},
         {},
         [&] { ++armed_gate_adjustments; },
         {}});
    if (!armed_gate_result.closed_by_armed_gate ||
        armed_gate_result.closed_by_controller ||
        armed_gate_result.exit_marker_set || armed_gate_result.iterations != 1U ||
        armed_gate_adjustments != 1) {
        return 44;
    }

    return 0;
}
