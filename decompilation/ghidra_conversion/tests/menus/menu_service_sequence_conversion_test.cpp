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
        empty_titles != 2) {
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

    InventoryGlyphRowSlots glyph_slots{};
    glyph_slots[0].item_code = 1U;
    glyph_slots[1].item_code = 5U;
    glyph_slots[2].item_code = 0U;
    glyph_slots[3].item_code = 8U;
    glyph_slots[1].attributes.action_code = 0x10;
    glyph_slots[1].attributes.class_code = 0;
    glyph_slots[1].attributes.equipped_slot = -1;
    const auto glyph_plan = render_inventory_glyph_row(glyph_slots);
    if (glyph_plan[0].destination != 0xC784U ||
        glyph_plan[1].destination != 0xC78EU ||
        glyph_plan[2].destination != 0xC904U ||
        glyph_plan[3].destination != 0xC90EU ||
        glyph_plan[4].destination != 0xCA84U ||
        glyph_plan[5].destination != 0xCA8EU ||
        glyph_plan[6].destination != 0xCC04U ||
        glyph_plan[7].destination != 0xCC0EU ||
        glyph_plan[0].glyph_source != 0x000D17DCU ||
        glyph_plan[1].glyph_source != 0x000D17E8U ||
        glyph_plan[2].glyph_source != kEmptyInventoryGlyphSource ||
        glyph_plan[3].glyph_source != 0x000D17F4U ||
        glyph_plan[0].attribute_flags != 0x2000U ||
        glyph_plan[1].attribute_flags != 0x6000U) {
        return 50;
    }
    std::vector<std::uint16_t> glyph_dests;
    render_inventory_glyph_row(
        glyph_slots, [&](const InventoryGlyphWrite& write) {
            glyph_dests.push_back(write.destination);
        });
    if (glyph_dests !=
        std::vector<std::uint16_t>{0xC784U, 0xC78EU, 0xC904U, 0xC90EU, 0xCA84U,
                                   0xCA8EU, 0xCC04U, 0xCC0EU}) {
        return 51;
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
    const auto slot0 = write_inventory_highlight_grid(0U);
    const auto slot2 = write_inventory_highlight_grid(2U);
    const auto slot4 = write_inventory_highlight_grid(4U);
    const auto slot7 = write_inventory_highlight_grid(7U);
    if (slot0.glyph_source != kInventoryHighlightGlyphEven ||
        slot0.destination != 0xE702U || slot0.height != 4U ||
        slot0.width != 3U ||
        slot2.glyph_source != kInventoryHighlightGlyphOdd ||
        slot2.destination != 0xC882U ||
        slot4.glyph_source != kInventoryHighlightGlyphEven ||
        slot4.destination != 0xEA02U ||
        slot7.glyph_source != kInventoryHighlightGlyphOdd ||
        slot7.destination != 0xCB8CU) {
        return 52;
    }
    const auto quad = write_menu_grid_quad();
    if (quad.grids[0].destination != 0xE702U ||
        quad.grids[1].destination != 0xE70CU ||
        quad.grids[2].destination != 0xEA0CU ||
        quad.grids[3].destination != 0xEA02U ||
        quad.grids[0].glyph_source != 0x000CD570U ||
        quad.metadata.destination != 0xC882U ||
        quad.metadata.height != 9U || quad.metadata.width != 9U ||
        quad.metadata.palette != 0x6000U ||
        quad.metadata.glyph_source != 0x000C1320U) {
        return 53;
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
    const auto dialog_plan = prepare_menu_dialog();
    if (dialog_plan.first_bar_destination != 0xC69CU ||
        dialog_plan.first_bar_selector != 0 ||
        dialog_plan.second_bar_destination != 0xC79CU ||
        dialog_plan.second_bar_selector != -1) {
        return 55;
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
    const auto text_plan = refresh_menu_text_plan();
    if (text_plan.name_destination != 0xC51CU ||
        text_plan.name_source_offset != 0x58U ||
        text_plan.second_source != 0x000D1AC6U ||
        text_plan.palette != 0x6000U) {
        return 56;
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
    const auto cursor_plan = advance_menu_text_cursor_plan();
    if (cursor_plan.name_destination != 0xC59EU ||
        cursor_plan.name_source_offset != 0x58U ||
        cursor_plan.hud_destination != 0xC928U ||
        cursor_plan.palette != 0x6000U) {
        return 58;
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
    MenuNumberRenderState wrap_zero{1, 5};
    render_menu_number(wrap_zero, -1, {{}, {}});
    MenuNumberRenderState wrap_negative{1, 5};
    render_menu_number(wrap_negative, -2, {{}, {}});
    const auto number_plan = menu_number_render_plan();
    if (wrap_zero.cursor != 5 ||
        wrap_negative.cursor != static_cast<std::int16_t>(0xFF05) ||
        number_plan.vram_command != 0xC4B0U ||
        number_plan.glyph_destination != 0xC4B2U) {
        return 59;
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
        panel.grid.destination != 0xEBAEU ||
        panel.grid.glyph_source != kMenuToggleGridGlyphBase + 8U ||
        panel.text_source != kMenuPanelTextBase + 0x12U ||
        panel_events != std::vector<int>{23, 24, 25}) {
        return 16;
    }
    refresh_menu_panel(panel, {}, {}, {});
    if (panel.toggle != 0x12U ||
        panel.grid.destination != 0xEBA8U ||
        panel.grid.glyph_source != kMenuToggleGridGlyphBase ||
        panel.text_source != kMenuPanelTextBase) {
        return 48;
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
    const auto variant_a = prepare_menu_grid_variant_a();
    const auto variant_b = prepare_menu_grid_variant_b();
    const auto variant_c = prepare_menu_grid_variant_c();
    if (variant_a.destination != kMenuGridVariantADestination ||
        variant_a.count != kMenuGridVariantACount ||
        variant_a.palette != 0x6000U ||
        variant_a.glyph_source != 0x000C1320U ||
        variant_b.destination != kMenuGridVariantBDestination ||
        variant_b.count != kMenuGridVariantBCount ||
        variant_c.destination != kMenuGridVariantCDestination ||
        variant_c.count != kMenuGridVariantCCount ||
        prepare_menu_grid_variant_d().destination !=
            kMenuGridVariantDDestination ||
        prepare_menu_grid_variant_d().count != kMenuGridVariantDCount) {
        return 49;
    }
    const auto row_grid = prepare_menu_row_grid();
    const auto row_render = render_menu_row();
    const auto repeated = write_menu_repeated_grid(0xCB18U, 0U);
    if (row_grid.title.destination != 0xCB18U ||
        row_grid.title.height != 0x14U || row_grid.title.width != 0U ||
        row_grid.title.palette != 0x6000U ||
        row_grid.body.destination != 0xEA18U || row_grid.body.width != 2U ||
        row_grid.body.palette != 0U ||
        row_render.glyph_source != 0x000CD48AU ||
        row_render.destination != 0x0000EA18U ||
        row_render.height != 0x14U || row_render.width != 2U ||
        row_render.posture_source != 0x000CF384U ||
        repeated.height != kMenuRepeatedGridHeight ||
        repeated.destination != 0xCB18U || repeated.palette != 0x6000U) {
        return 54;
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
    const auto text_asset = render_menu_text_asset();
    if (text_asset.destination != 0xC69CU ||
        text_asset.source != 0x000D19A8U || text_asset.palette != 0x6000U) {
        return 57;
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

    CommandOverviewState overview_state;
    std::vector<std::uint16_t> overview_events{0x0800U, 0x4000U, 0x8000U};
    std::size_t overview_event_index = 0U;
    std::vector<std::string_view> overview_uploads;
    std::vector<std::uint16_t> overview_display_controls;
    std::vector<std::uint16_t> overview_audio;
    std::vector<std::uint8_t> overview_titles;
    std::vector<std::int8_t> overview_page_deltas;
    int overview_mode_actions = 0;
    int overview_prepares = 0;
    int overview_palette_refreshes = 0;
    int overview_font_loads = 0;
    int overview_panels = 0;
    int overview_names = 0;
    int overview_rows = 0;
    int overview_highlights = 0;
    int overview_vblanks = 0;
    int overview_latches = 0;
    int overview_details = 0;
    int overview_direct_detail_refreshes = 0;
    int overview_inventory_actions = 0;
    const auto overview_result = run_command_overview(
        overview_state,
        CommandOverviewInput{0x50U, 0},
        [&] {
            CommandOverviewCallbacks callbacks;
            callbacks.handle_mode_action_event = [&] { ++overview_mode_actions; };
            callbacks.set_display_control = [&](const std::uint16_t control) {
                overview_display_controls.push_back(control);
            };
            callbacks.prepare_command_overview = [&] { ++overview_prepares; };
            callbacks.refresh_mode_palette_data =
                [&] { ++overview_palette_refreshes; };
            callbacks.load_title_font = [&] { ++overview_font_loads; };
            callbacks.draw_hud_panel = [&] { ++overview_panels; };
            callbacks.render_active_runner_name = [&] { ++overview_names; };
            callbacks.render_menu_row = [&] { ++overview_rows; };
            callbacks.render_equipment_highlights = [&] { ++overview_highlights; };
            callbacks.upload_menu_tile_block =
                [&](const CommandOverviewTileUploadPlan& plan) {
                    overview_uploads.push_back(plan.asset_alias);
                };
            callbacks.reset_upload_cursor_and_upload =
                [&](const CommandOverviewTileUploadPlan& plan) {
                    overview_uploads.push_back(plan.asset_alias);
                };
            callbacks.wait_for_vblank = [&] { ++overview_vblanks; };
            callbacks.latch_controller_events = [&] { ++overview_latches; };
            callbacks.sample_controller_events =
                [&] { return overview_events.at(overview_event_index++); };
            callbacks.service_audio = [&](const std::uint16_t event) {
                overview_audio.push_back(event);
            };
            callbacks.select_title_text =
                [&](const std::uint8_t title) { overview_titles.push_back(title); };
            callbacks.render_equipment_details = [&] { ++overview_details; };
            callbacks.advance_primary_page =
                [&](CommandOverviewState& state, const std::int8_t delta) {
                    overview_page_deltas.push_back(delta);
                    state.primary_page = static_cast<std::uint8_t>(
                        (static_cast<int>(state.primary_page) + delta + 7) % 7);
                };
            callbacks.advance_secondary_page =
                [&](CommandOverviewState&, const std::int8_t delta) {
                    overview_page_deltas.push_back(delta);
                };
            callbacks.run_inventory_actions =
                [&](CommandOverviewState&) { ++overview_inventory_actions; };
            callbacks.refresh_action_details =
                [&] { ++overview_direct_detail_refreshes; };
            return callbacks;
        }());
    if (overview_result.exit_reason != CommandOverviewExitReason::controller ||
        overview_result.controller_iterations != 3U ||
        overview_result.detail_refreshes != 3U ||
        overview_result.full_entry_initializations != 1U ||
        overview_result.partial_refreshes != 0U ||
        overview_result.inventory_actions != 1U ||
        !overview_result.mode_action_handled_on_entry ||
        !overview_result.mode_action_handled_on_exit || overview_mode_actions != 2 ||
        overview_prepares != 1 || overview_palette_refreshes != 1 ||
        overview_font_loads != 1 || overview_panels != 1 || overview_names != 1 ||
        overview_rows != 1 || overview_highlights != 1 || overview_vblanks != 3 ||
        overview_latches != 3 || overview_details != 2 ||
        overview_direct_detail_refreshes != 1 || overview_inventory_actions != 1 ||
        overview_display_controls != std::vector<std::uint16_t>{0x8134U, 0x8174U} ||
        overview_audio != std::vector<std::uint16_t>{0x25U, 0x25U, 0x24U} ||
        overview_titles != std::vector<std::uint8_t>{6U, 0U} ||
        overview_page_deltas != std::vector<std::int8_t>{1} ||
        overview_uploads !=
            std::vector<std::string_view>{"menu.field.command_overview.tile_block",
                                          "menu.field.command_overview.cursor_block"} ||
        overview_state.primary_page != 0U || overview_state.action_marker != 0 ||
        overview_state.exit_marker != 0 || overview_inventory_actions != 1) {
        return 45;
    }

    CommandOverviewState dialog_exit_state;
    std::size_t dialog_exit_event_index = 0U;
    const auto dialog_exit_result = run_command_overview(
        dialog_exit_state, CommandOverviewInput{},
        [&] {
            CommandOverviewCallbacks callbacks;
            callbacks.sample_controller_events = [&] {
                ++dialog_exit_event_index;
                return static_cast<std::uint16_t>(0x0200U);
            };
            callbacks.advance_secondary_page =
                [&](CommandOverviewState& state, const std::int8_t) {
                    state.exit_marker = -1;
                };
            return callbacks;
        }());
    if (dialog_exit_result.exit_reason !=
            CommandOverviewExitReason::action_dialog ||
        dialog_exit_result.controller_iterations != 1U ||
        dialog_exit_state.exit_marker != 0 || dialog_exit_event_index != 1U) {
        return 46;
    }

    CommandOverviewPreparationInput preparation_input;
    for (std::size_t group = 0U;
         group < preparation_input.repeated_assets.groups.size(); ++group) {
        preparation_input.repeated_assets.groups[group].uses_entity_asset_table =
            group == 1U;
        for (std::size_t entry = 0U;
             entry < preparation_input.repeated_assets.groups[group]
                         .asset_indices.size();
             ++entry) {
            preparation_input.repeated_assets.groups[group].asset_indices[entry] =
                static_cast<std::uint16_t>(group * 100U + entry);
        }
    }
    preparation_input.alternate_resource_variant = true;
    std::vector<CommandOverviewGridPlan> preparation_grids;
    std::vector<CommandOverviewRepeatedAssetPlan> repeated_assets;
    std::vector<CommandOverviewResourceUploadPlan> resource_uploads;
    std::vector<CommandOverviewBlockTransferPlan> preparation_blocks;
    int preparation_graphics = 0;
    const auto preparation_result = prepare_command_overview(
        preparation_input,
        {[&] { ++preparation_graphics; },
         [&](const CommandOverviewGridPlan& plan) {
             preparation_grids.push_back(plan);
         },
         [&](const CommandOverviewRepeatedAssetPlan& plan) {
             repeated_assets.push_back(plan);
         },
         [&](const CommandOverviewResourceUploadPlan& plan) {
             resource_uploads.push_back(plan);
         },
         [&](const CommandOverviewBlockTransferPlan& plan) {
             preparation_blocks.push_back(plan);
         }});
    if (preparation_result.primary_page != 6U ||
        preparation_result.secondary_page != 0U ||
        preparation_result.resource_variant !=
            CommandOverviewResourceVariant::alternate ||
        preparation_result.grid_writes != 4U ||
        preparation_result.repeated_asset_writes != 240U ||
        preparation_result.resource_uploads != 1U ||
        preparation_result.block_writes != 3U ||
        !preparation_result.refresh_marker_cleared || preparation_graphics != 1 ||
        preparation_grids.size() != 4U ||
        preparation_grids[0].destination_tile_base != 0xE000U ||
        preparation_grids[0].rows != 28U ||
        preparation_grids[0].words_per_row != 12U ||
        preparation_grids[1].destination_tile_base != 0xE018U ||
        preparation_grids[2].destination_tile_base != 0xE042U ||
        preparation_grids[3].destination_tile_base != 0xEB98U ||
        repeated_assets.size() != 240U || repeated_assets[0].group_index != 0U ||
        repeated_assets[0].entry_index != 0U ||
        repeated_assets[0].asset_index != 0U ||
        repeated_assets[0].destination_offset != 0x20U ||
        repeated_assets[79].destination_offset != 0xA00U ||
        !repeated_assets[80].uses_entity_asset_table ||
        repeated_assets[80].asset_index != 100U ||
        repeated_assets[239].asset_index != 279U || resource_uploads.size() != 1U ||
        resource_uploads[0].variant != CommandOverviewResourceVariant::alternate ||
        resource_uploads[0].asset_alias !=
            "menu.field.command_overview.resource.alternate" ||
        resource_uploads[0].byte_count != 0x5C80U ||
        preparation_blocks.size() != 3U ||
        preparation_blocks[0].asset_alias !=
            "menu.field.command_overview.panel_tiles" ||
        preparation_blocks[0].destination != 0x7D20U ||
        preparation_blocks[0].byte_count != 0x1E00U ||
        preparation_blocks[1].asset_alias !=
            "menu.field.command_overview.detail_tiles" ||
        preparation_blocks[1].destination != 0x9B20U ||
        preparation_blocks[1].byte_count != 0x420U ||
        preparation_blocks[2].asset_alias !=
            "menu.field.command_overview.background" ||
        preparation_blocks[2].destination != 0x5A80U ||
        preparation_blocks[2].byte_count != 0x200U) {
        return 47;
    }

    return 0;
}
