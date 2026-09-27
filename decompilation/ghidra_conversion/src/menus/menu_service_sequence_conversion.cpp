#include "shadowrun_ghidra/menus/menu_service_sequence_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

bool cmpi_byte_result_is_nonnegative(const std::int8_t value,
                                    const std::uint8_t immediate) {
    const auto raw_value = static_cast<std::uint8_t>(value);
    const auto result = static_cast<std::uint8_t>(raw_value - immediate);
    return (result & 0x80U) == 0U;
}

} // namespace

void draw_command_overview_hud_panel(
    const CommandOverviewHudPanelInput& input,
    const std::function<void(const CommandOverviewHudPanelPlan&)>& draw_panel) {
    if (!draw_panel) {
        return;
    }

    const auto tile_word = static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(input.panel_variant) * 0x50U + 0x8001U);
    CommandOverviewHudPanelPlan plan;
    plan.tile_origin = static_cast<std::int16_t>(tile_word);
    plan.field_parameter = input.field_parameter;
    plan.destination_tile_base = 0xE104U;
    draw_panel(plan);
}

void draw_active_runner_hud_panel(
    const ActiveRunnerHudPanelInput& input,
    const std::function<void(const ActiveRunnerHudPanelPlan&)>& draw_panel) {
    if (draw_panel) {
        ActiveRunnerHudPanelPlan plan;
        plan.field_parameter = input.field_parameter;
        plan.destination_tile_base = input.destination_tile_base;
        plan.tile_origin = input.panel_variant > 1
                               ? static_cast<std::int16_t>(-0x7940)
                               : input.panel_variant == 1
                                     ? static_cast<std::int16_t>(-0x7956)
                                     : static_cast<std::int16_t>(-0x796C);
        draw_panel(plan);
    }
}

void render_active_runner_name_text(
    const std::function<void(const ActiveRunnerNameTextPlan&)>& render_text) {
    if (render_text) {
        render_text(ActiveRunnerNameTextPlan{});
    }
}

CommandOverviewActionRefreshResult refresh_command_overview_action_details(
    CommandOverviewActionRefreshState& state,
    const CommandOverviewActionRefreshCallbacks& callbacks) {
    CommandOverviewActionRefreshResult result;
    result.sprite_plan.y_offset = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(state.primary_page) * 0x28U);
    result.sprite_plan.x_offset = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(state.secondary_page) * 0x18U);

    // ROM 0x115AE initializes the presentation-record count before entering
    // 0x1378. The callback returns the typed records appended by that build.
    state.sprite_record_count = 1U;
    if (callbacks.build_title_sprite_records) {
        const auto built_count =
            callbacks.build_title_sprite_records(result.sprite_plan);
        state.sprite_record_count = static_cast<std::uint16_t>(
            1U + static_cast<std::uint16_t>(built_count));
    }
    if (callbacks.clear_last_sprite_sequence) {
        callbacks.clear_last_sprite_sequence();
    }
    state.sprite_list_ready = true;
    state.display_dirty_flags = static_cast<std::uint8_t>(
        state.display_dirty_flags | 0x02U);

    if (callbacks.prepare_menu_grid_variant_b) {
        callbacks.prepare_menu_grid_variant_b();
    }

    if (callbacks.read_entity_action_code) {
        result.action_code = callbacks.read_entity_action_code();
    }
    if (result.action_code != 0U) {
        result.item_details_rendered = true;
        result.item_details_result = result.action_code;
        if (callbacks.render_item_action_details) {
            callbacks.render_item_action_details();
        }

        // CMP.B #0x10 followed by BPL at 0x115F6 calls the entity-action
        // service only when the signed byte subtraction is negative. Keeping
        // the wrapped byte test avoids silently turning the source's byte
        // condition into a host integer comparison.
        const auto comparison = static_cast<std::uint8_t>(
            result.item_details_result - 0x10U);
        if ((comparison & 0x80U) != 0U) {
            if (callbacks.service_entity_action_menu) {
                callbacks.service_entity_action_menu();
                result.action_menu_serviced = true;
            }
        }
        if (callbacks.prepare_menu_dialog) {
            callbacks.prepare_menu_dialog();
            result.dialog_prepared = true;
        }
    } else if (callbacks.render_action_title_text) {
        callbacks.render_action_title_text(CommandOverviewActionTitleTextPlan{});
        result.action_title_rendered = true;
    }

    return result;
}

std::optional<MenuTitleTextPlan> select_menu_title_text(
    const MenuTitleTextInput& input) {
    MenuTitleTextPlan plan;
    if (input.selector < 2U) {
        plan.source_alias = "menu.inventory.labels";
        plan.record_index = 0U;
        plan.used_inventory_fallback = true;
    } else {
        if (input.caller_table.alias.empty()) {
            return std::nullopt;
        }
        plan.source_alias = input.caller_table.alias;
        plan.record_index = static_cast<std::size_t>(input.selector - 2U);
    }
    plan.record_offset_bytes = plan.record_index * 0x12U;
    return plan;
}

std::optional<MenuTitleTextPlan> select_command_overview_title_text(
    const std::uint8_t selector) {
    return select_menu_title_text(
        MenuTitleTextInput{selector, MenuTextTableRef{"menu.command.labels"}});
}

std::uint16_t select_hud_glyph_attribute_flags(
    const HudGlyphAttributeInput& input) {
    if (input.action_code == 0) {
        return 0x2000U;
    }

    // CMP.B #2,record+0x9B / BPL takes the nonnegative branch at 0x362A.
    // TST.B record+0x56 / BPL takes the same branch when the slot sign bit
    // is clear. The first branch therefore requires both negative flags.
    if (!cmpi_byte_result_is_nonnegative(input.class_code, 2U) &&
        input.equipped_slot < 0) {
        // CMP.B #6,record+0x57 / BPL selects 0x6000; a negative result
        // falls through to the 0x2000 return.
        return cmpi_byte_result_is_nonnegative(input.action_code, 6U)
                   ? 0x6000U
                   : 0x2000U;
    }

    // CMP.B #0x0F / BMI selects 0x6000; the nonnegative result selects
    // 0x2000.
    return cmpi_byte_result_is_nonnegative(input.action_code, 0x0FU)
               ? 0x2000U
               : 0x6000U;
}

std::uint16_t render_menu_tile_rows(
    const std::function<void()>& clear_row,
    const std::function<void()>& write_row) {
    std::uint16_t countdown = 7U;
    std::uint16_t last_countdown = 0U;
    do {
        if (clear_row) {
            clear_row();
        }
        if (write_row) {
            write_row();
        }
        last_countdown = countdown;
        countdown = static_cast<std::uint16_t>(countdown - 1U);
    } while (countdown != 0xFFFFU);
    return static_cast<std::uint16_t>(last_countdown >> 1U);
}

std::int8_t reset_and_upload_menu_tile_block(
    const std::function<void()>& reset_upload_state,
    const std::function<void()>& upload_tile_block,
    const std::int8_t preserved_d0) {
    if (reset_upload_state) {
        reset_upload_state();
    }
    if (upload_tile_block) {
        upload_tile_block();
    }
    return preserved_d0;
}

void advance_command_overview_primary_page(
    CommandOverviewPrimaryPageState& state,
    const CommandOverviewPrimaryPageCallbacks& callbacks) {
    const auto delta = callbacks.reset_and_upload_menu_tile_block
                           ? callbacks.reset_and_upload_menu_tile_block()
                           : std::int8_t{0};
    const auto updated_page = static_cast<std::uint8_t>(
        static_cast<std::uint8_t>(state.page) +
        static_cast<std::uint8_t>(delta));
    state.page = updated_page;

    // The source uses ADD.B followed by BPL, so the sign test is on the
    // wrapped byte rather than on a widened host integer.
    if ((state.page & 0x80U) != 0U) {
        state.auxiliary_marker = -1;
        state.page = 6U;
    }
    if (state.page == 7U) {
        state.page = 0U;
    }
    if (state.page == 2U && delta >= 0) {
        state.auxiliary_marker = -1;
    }

    if (state.page >= 2U) {
        if (callbacks.reset_vdp_upload_state) {
            callbacks.reset_vdp_upload_state();
        }
        if (callbacks.reset_menu_upload_cursor_and_upload) {
            callbacks.reset_menu_upload_cursor_and_upload();
        }
        return;
    }
    if (callbacks.refresh_action_details) {
        callbacks.refresh_action_details();
    }
}

void advance_command_overview_secondary_page(
    CommandOverviewSecondaryPageState& state,
    const CommandOverviewSecondaryPageCallbacks& callbacks) {
    if (state.primary_page >= 2U) {
        const auto navigation_mask = static_cast<std::uint16_t>(
            (1U << 1U) | (1U << 3U) | (1U << 2U));
        if ((state.controller_events & navigation_mask) == 0U) {
            if (state.action_dialog_gate >= 0) {
                if (callbacks.reset_and_upload_menu_tile_block) {
                    callbacks.reset_and_upload_menu_tile_block();
                }
                state.action_dialog_gate = -1;
                if (callbacks.run_action_dialog) {
                    callbacks.run_action_dialog();
                }
                state.action_dialog_gate = 0;
                if (callbacks.reset_menu_upload_cursor_and_upload) {
                    callbacks.reset_menu_upload_cursor_and_upload();
                }
                return;
            }
            state.action_dialog_gate = 0;
        }
        return;
    }

    const auto delta = callbacks.reset_and_upload_menu_tile_block
                           ? callbacks.reset_and_upload_menu_tile_block()
                           : std::int8_t{0};
    state.secondary_page = static_cast<std::uint8_t>(
        static_cast<std::uint8_t>(state.secondary_page) +
        static_cast<std::uint8_t>(delta));
    if ((state.secondary_page & 0x80U) != 0U) {
        state.secondary_page = 0U;
        if (callbacks.refresh_action_details) {
            callbacks.refresh_action_details();
        }
        return;
    }
    if (state.secondary_page == 4U) {
        state.secondary_page = 3U;
    }
    if (callbacks.refresh_action_details) {
        callbacks.refresh_action_details();
    }
}

std::size_t write_command_overview_repeated_asset_batch(
    const CommandOverviewRepeatedAssetInput& input,
    const std::function<void(const CommandOverviewRepeatedAssetPlan&)>&
        write_asset) {
    std::size_t writes = 0U;
    for (std::size_t group_index = 0U; group_index < input.groups.size();
         ++group_index) {
        const auto& group = input.groups[group_index];
        for (std::size_t entry_index = 0U;
             entry_index < group.asset_indices.size(); ++entry_index) {
            if (write_asset) {
                write_asset(CommandOverviewRepeatedAssetPlan{
                    group_index,
                    entry_index,
                    group.asset_indices[entry_index],
                    static_cast<std::uint16_t>(0x20U + entry_index * 0x20U),
                    0x20U,
                    group.uses_entity_asset_table});
            }
            ++writes;
        }
    }
    return writes;
}

void upload_command_overview_resource(
    const bool alternate_resource_variant,
    const std::function<void(const CommandOverviewResourceUploadPlan&)>&
        upload_resource) {
    if (!upload_resource) {
        return;
    }
    upload_resource({
        alternate_resource_variant ? CommandOverviewResourceVariant::alternate
                                   : CommandOverviewResourceVariant::primary,
        alternate_resource_variant
            ? "menu.field.command_overview.resource.alternate"
            : "menu.field.command_overview.resource.primary",
        0x5C80U});
}

std::size_t upload_command_overview_resource_and_blocks(
    const bool alternate_resource_variant,
    const CommandOverviewPreparationCallbacks& callbacks) {
    upload_command_overview_resource(alternate_resource_variant,
                                     callbacks.upload_resource_stream);
    constexpr std::array<CommandOverviewBlockTransferPlan, 2> blocks{
        CommandOverviewBlockTransferPlan{
            "menu.field.command_overview.panel_tiles", 0x7D20U, 0x1E00U},
        CommandOverviewBlockTransferPlan{
            "menu.field.command_overview.detail_tiles", 0x9B20U, 0x0420U}};
    for (const auto& block : blocks) {
        if (callbacks.write_block) {
            callbacks.write_block(block);
        }
    }
    return blocks.size();
}

CommandOverviewPreparationResult prepare_command_overview(
    const CommandOverviewPreparationInput& input,
    const CommandOverviewPreparationCallbacks& callbacks) {
    CommandOverviewPreparationResult result;
    result.resource_variant = input.alternate_resource_variant
                                  ? CommandOverviewResourceVariant::alternate
                                  : CommandOverviewResourceVariant::primary;

    if (callbacks.prepare_menu_graphics) {
        callbacks.prepare_menu_graphics();
    }

    // ROM 0x10F6E..0x10FCC. The source D7/D1 loop values are expressed as
    // host row/word counts, so the +1 DBF convention is no longer implicit.
    constexpr std::array<CommandOverviewGridPlan, 4> grids{
        CommandOverviewGridPlan{
            "menu.field.command_overview.layout", 0xE000U, 28U, 12U, 0U},
        CommandOverviewGridPlan{
            "menu.field.command_overview.panel_frame", 0xE018U, 2U, 21U, 0U},
        CommandOverviewGridPlan{
            "menu.field.command_overview.action_grid", 0xE042U, 23U, 1U, 0U},
        CommandOverviewGridPlan{
            "menu.field.command_overview.detail_grid", 0xEB98U, 5U, 22U, 0U}};
    for (const auto& grid : grids) {
        if (callbacks.write_grid) {
            callbacks.write_grid(grid);
        }
        ++result.grid_writes;
    }

    result.repeated_asset_writes = write_command_overview_repeated_asset_batch(
        input.repeated_assets, callbacks.write_repeated_asset);

    result.resource_uploads = 1U;
    result.block_writes = upload_command_overview_resource_and_blocks(
        input.alternate_resource_variant,
        CommandOverviewPreparationCallbacks{
            {}, {}, {}, callbacks.upload_resource_stream, callbacks.write_block});

    if (callbacks.write_block) {
        callbacks.write_block(
            {"menu.field.command_overview.background", 0x5A80U, 0x0200U});
    }
    ++result.block_writes;
    result.refresh_marker_cleared = true;
    return result;
}

CommandOverviewRunResult run_command_overview(
    CommandOverviewState& state,
    const CommandOverviewInput& input,
    const CommandOverviewCallbacks& callbacks) {
    CommandOverviewRunResult result;

    const auto mode_action_required = [&] {
        return (input.active_mode_flags & 0x50U) != 0U;
    };

    const auto render_panel_pass = [&] {
        // ROM 0x10D90 is shared by the initial entry and every partial panel
        // refresh. The two upload boundaries receive host aliases instead of
        // the source A1 values 0xCD2C6 and 0xCD3F2.
        state.panel_marker = 0;
        if (callbacks.draw_hud_panel) {
            callbacks.draw_hud_panel();
        }
        if (callbacks.render_active_runner_name) {
            callbacks.render_active_runner_name();
        }
        if (callbacks.render_menu_row) {
            callbacks.render_menu_row();
        }
        if (callbacks.render_equipment_highlights) {
            callbacks.render_equipment_highlights();
        }
        if (callbacks.upload_menu_tile_block) {
            callbacks.upload_menu_tile_block(
                {"menu.field.command_overview.tile_block"});
        }
        if (callbacks.reset_upload_cursor_and_upload) {
            callbacks.reset_upload_cursor_and_upload(
                {"menu.field.command_overview.cursor_block"});
        }
        state.refresh_marker = -1;
    };

    const auto run_full_entry = [&] {
        if (mode_action_required() && callbacks.handle_mode_action_event) {
            callbacks.handle_mode_action_event();
            result.mode_action_handled_on_entry = true;
        }

        state.primary_page = 6U;
        state.secondary_page = 0U;
        state.action_marker = -1;
        state.exit_marker = 0;
        state.panel_marker = 0;
        state.graphics_marker = 0;
        state.resource_variant = 0;
        if (callbacks.set_display_control) {
            callbacks.set_display_control(0x8134U);
        }
        if (callbacks.prepare_command_overview) {
            callbacks.prepare_command_overview();
        }
        if (callbacks.refresh_mode_palette_data) {
            callbacks.refresh_mode_palette_data();
        }
        if (callbacks.load_title_font) {
            callbacks.load_title_font();
        }
        if (callbacks.set_display_control) {
            callbacks.set_display_control(0x8174U);
        }
        render_panel_pass();
        ++result.full_entry_initializations;
    };

    const auto run_detail_cycle = [&] {
        if (callbacks.service_audio) {
            callbacks.service_audio(0x25U);
        }
        if (callbacks.select_title_text) {
            callbacks.select_title_text(state.primary_page);
        }
        if (callbacks.render_equipment_details) {
            callbacks.render_equipment_details();
        }
        ++result.detail_refreshes;
    };

    run_full_entry();
    run_detail_cycle();

    bool finished = false;
    while (!finished) {
        if (callbacks.wait_for_vblank) {
            callbacks.wait_for_vblank();
        }
        if (callbacks.latch_controller_events) {
            callbacks.latch_controller_events();
        }
        if (!callbacks.sample_controller_events) {
            result.exit_reason = CommandOverviewExitReason::controller_input_missing;
            return result;
        }

        const auto events = callbacks.sample_controller_events();
        ++result.controller_iterations;

        // 0x10DD8..0x10E20: horizontal and vertical page edges have source
        // priority over inventory, mode, and exit events.
        if ((events & 0x0400U) != 0U) {
            if (callbacks.advance_primary_page) {
                callbacks.advance_primary_page(state, -1);
            }
            run_detail_cycle();
            continue;
        }
        if ((events & 0x0100U) != 0U) {
            if (callbacks.advance_secondary_page) {
                callbacks.advance_secondary_page(state, -1);
            }
            if (state.exit_marker != 0) {
                result.exit_reason = CommandOverviewExitReason::action_dialog;
                finished = true;
            } else {
                run_detail_cycle();
            }
            continue;
        }
        if ((events & 0x0800U) != 0U) {
            if (callbacks.advance_primary_page) {
                callbacks.advance_primary_page(state, 1);
            }
            run_detail_cycle();
            continue;
        }
        if ((events & 0x0200U) != 0U) {
            if (callbacks.advance_secondary_page) {
                callbacks.advance_secondary_page(state, 1);
            }
            if (state.exit_marker != 0) {
                result.exit_reason = CommandOverviewExitReason::action_dialog;
                finished = true;
            } else {
                run_detail_cycle();
            }
            continue;
        }

        // 0x10E42 and 0x10EC8 merge the two inventory/action buttons into one
        // source path. The audio event and saved primary page are shared.
        if ((events & 0x4000U) != 0U || (events & 0x1000U) != 0U) {
            if (callbacks.service_audio) {
                callbacks.service_audio(0x24U);
            }
            const auto saved_primary_page = state.primary_page;
            if (callbacks.run_inventory_actions) {
                callbacks.run_inventory_actions(state);
            }
            state.primary_page = saved_primary_page;
            ++result.inventory_actions;

            if (state.primary_page < 2U) {
                if (callbacks.refresh_action_details) {
                    callbacks.refresh_action_details();
                }
                ++result.detail_refreshes;
                continue;
            }
            if (state.primary_page == 3U) {
                run_full_entry();
                run_detail_cycle();
                continue;
            }
            if (state.primary_page == 6U && state.inventory_exit_marker != 0) {
                state.inventory_exit_marker = 0;
                result.exit_reason = CommandOverviewExitReason::inventory;
                finished = true;
                continue;
            }
            if (input.active_class_code > 1 || state.primary_page != 5U) {
                render_panel_pass();
                ++result.partial_refreshes;
            } else {
                run_full_entry();
                run_detail_cycle();
            }
            continue;
        }

        if ((events & 0x2000U) != 0U) {
            if (callbacks.handle_mode_action_event) {
                callbacks.handle_mode_action_event();
            }
            if (callbacks.reset_vdp_upload_state) {
                callbacks.reset_vdp_upload_state();
            }
            state.primary_page = 6U;
            state.secondary_page = 0U;
            if (callbacks.advance_primary_page) {
                callbacks.advance_primary_page(state, 0);
            }
            state.action_dialog_gate = -1;
            if (callbacks.advance_secondary_page) {
                callbacks.advance_secondary_page(state, 0);
            }
            render_panel_pass();
            ++result.partial_refreshes;
            continue;
        }

        if ((events & 0x8000U) != 0U) {
            result.exit_reason = CommandOverviewExitReason::controller;
            finished = true;
        }
    }

    if (mode_action_required() && callbacks.handle_mode_action_event) {
        callbacks.handle_mode_action_event();
        result.mode_action_handled_on_exit = true;
    }
    state.action_marker = 0;
    state.exit_marker = 0;
    return result;
}

void write_menu_grid(const std::function<void()>& write_grid) {
    if (write_grid) {
        write_grid();
    }
}

void write_menu_grid_quad(const std::function<void()>& write_grid,
                          const std::function<void()>& refresh_metadata) {
    for (int index = 0; index < 4; ++index) {
        if (write_grid) {
            write_grid();
        }
    }
    if (refresh_metadata) {
        refresh_metadata();
    }
}

void prepare_menu_row_grid(const std::function<void()>& prepare_row,
                           const std::function<void()>& write_repeated_grid) {
    if (prepare_row) {
        prepare_row();
    }
    if (write_repeated_grid) {
        write_repeated_grid();
    }
}

void render_menu_row(const std::function<void()>& write_grid,
                     const std::function<void()>& render_row) {
    if (write_grid) {
        write_grid();
    }
    if (render_row) {
        render_row();
    }
}

std::uint64_t prepare_menu_dialog(
    const std::function<std::uint64_t()>& prepare_dialog,
    const std::function<void()>& service_dialog) {
    const auto result = prepare_dialog ? prepare_dialog() : 0U;
    if (service_dialog) {
        service_dialog();
        service_dialog();
    }
    return result;
}

bool refresh_menu_text(
    MenuTextRefreshState& state,
    const std::function<void()>& prepare_text,
    const std::function<void()>& render_text) {
    if (state.pending_text_count >= 1) {
        return false;
    }
    if (prepare_text) {
        prepare_text();
    }
    if (render_text) {
        render_text();
        render_text();
    }
    state.pending_text_count = 0;
    return true;
}

void advance_menu_text_cursor(
    MenuTextCursorState& state,
    const std::function<void()>& render_text,
    const std::function<void()>& service_cursor) {
    std::int16_t* selected_marker = nullptr;
    do {
        if (state.selector == 0) {
            state.selector = -1;
            selected_marker = &state.primary_marker;
        } else {
            state.selector = 0;
            selected_marker = &state.secondary_marker;
        }
    } while (*selected_marker < 0);

    if (render_text) {
        render_text();
    }
    if (service_cursor) {
        service_cursor();
    }
}

void render_menu_number(MenuNumberRenderState& state,
                        const std::int16_t delta,
                        const MenuNumberRenderCallbacks& callbacks) {
    if (callbacks.write_number_word) {
        callbacks.write_number_word(0x6000U);
    }

    const auto sum = static_cast<std::int32_t>(state.cursor) + delta;
    const bool overflow = sum > 0x7FFF || sum < -0x8000;
    state.cursor = static_cast<std::int16_t>(sum);
    if (state.cursor == 0 || overflow) {
        state.cursor = static_cast<std::int16_t>(state.available_count);
    }
    if (static_cast<std::int16_t>(state.available_count) < state.cursor) {
        state.cursor = 1;
    }
    if (callbacks.build_decimal_glyphs) {
        callbacks.build_decimal_glyphs(state.cursor);
    }
}

void clear_menu_slot(MenuSlotCleanupState& state,
                     const std::int8_t action,
                     const std::size_t slot_index,
                     const bool asset_load_pending,
                     const std::function<void()>& prepare_assets) {
    const auto index = slot_index % state.pending_actions.size();
    if (action < 0x2C) {
        if (asset_load_pending) {
            state.slot_type = 0x7FU;
            state.slot_subtype = 0U;
            state.action_marker = 0;
            if (prepare_assets) {
                prepare_assets();
            }
        }
    } else if (static_cast<std::int8_t>(action - 0x2B) ==
                   state.action_marker &&
               state.active_slots[index] != 0U) {
        state.action_marker = 0;
    }
    state.pending_actions[index] = 0U;
    state.active_slots[index] = 0U;
}

void refresh_menu_panel(MenuPanelRefreshState& state,
                        const std::function<void()>& upload_panel,
                        const std::function<void()>& update_layout,
                        const std::function<void()>& render_text) {
    if (upload_panel) {
        upload_panel();
    }
    state.toggle = static_cast<std::uint8_t>(state.toggle ^ 0xFFU);
    if (update_layout) {
        update_layout();
    }
    if (render_text) {
        render_text();
    }
}

void prepare_menu_grid_variant_a(
    const std::function<void()>& prepare_grid) {
    if (prepare_grid) {
        prepare_grid();
    }
}

void prepare_menu_grid_variant_b(
    const std::function<void()>& prepare_grid) {
    if (prepare_grid) {
        prepare_grid();
    }
}

void prepare_menu_grid_variant_c(
    const std::function<void()>& prepare_grid) {
    if (prepare_grid) {
        prepare_grid();
    }
}

void write_menu_repeated_grid(const std::function<void()>& write_grid) {
    if (write_grid) {
        write_grid();
    }
}

void write_menu_repeated_grid_with_metadata(
    const std::function<void()>& write_grid) {
    if (write_grid) {
        write_grid();
    }
}

void render_menu_text_asset(const std::function<void()>& render_text) {
    if (render_text) {
        render_text();
    }
}

CommandOverviewActionDialogResult run_command_overview_action_dialog(
    const CommandOverviewActionDialogInput& input,
    const CommandOverviewActionDialogCallbacks& callbacks) {
    CommandOverviewActionDialogResult result;

    const CommandOverviewActionDialogGridPlan initial_grid{
        CommandOverviewActionDialogGridKind::initial, 0xCA9EU, 0U, 5U, 0U};
    if (callbacks.write_grid) {
        callbacks.write_grid(initial_grid);
    }
    result.initial_grid_written = true;

    if (callbacks.prepare_dialog) {
        callbacks.prepare_dialog({0});
    }

    bool closed = false;
    while (!closed) {
        if (callbacks.wait_for_vblank) {
            callbacks.wait_for_vblank();
        }
        if (callbacks.latch_controller_events) {
            callbacks.latch_controller_events();
        }
        if (!callbacks.sample_controller_events) {
            result.controller_input_missing = true;
            break;
        }

        const std::uint16_t events = callbacks.sample_controller_events();
        ++result.iterations;
        bool check_increment = false;
        bool check_decrement = false;
        if ((events & 0x0200U) != 0U) {
            if ((events & 0x0008U) != 0U) {
                check_increment = true;
                if ((events & 0x0800U) != 0U) {
                    if (callbacks.service_audio) {
                        callbacks.service_audio(0x25U);
                    }
                    if (callbacks.prepare_dialog) {
                        callbacks.prepare_dialog({1});
                    }
                    result.posture_deltas.push_back(1);
                    continue;
                }
            } else if ((events & 0x0004U) != 0U) {
                check_decrement = true;
            } else if (input.dialog_gate < 0) {
                result.closed_by_armed_gate = true;
                closed = true;
                continue;
            } else {
                check_increment = true;
            }
        } else {
            check_increment = true;
        }

        if (check_increment) {
            if ((events & 0x0800U) != 0U) {
                if (callbacks.service_audio) {
                    callbacks.service_audio(0x25U);
                }
                if (callbacks.prepare_dialog) {
                    callbacks.prepare_dialog({1});
                }
                result.posture_deltas.push_back(1);
                continue;
            }
            check_decrement = true;
        }

        if (check_decrement && (events & 0x0400U) != 0U) {
            if (callbacks.service_audio) {
                callbacks.service_audio(0x25U);
            }
            if (callbacks.prepare_dialog) {
                callbacks.prepare_dialog({-1});
            }
            result.posture_deltas.push_back(-1);
            continue;
        }

        if ((events & 0x8000U) != 0U) {
            result.closed_by_controller = true;
            result.exit_marker_set = true;
            closed = true;
        }
    }

    const CommandOverviewActionDialogGridPlan final_grid{
        CommandOverviewActionDialogGridKind::final, 0xCA9EU, 0U, 5U, 0U};
    if (callbacks.write_grid) {
        callbacks.write_grid(final_grid);
    }
    result.final_grid_written = true;

    if (callbacks.adjust_dialog_posture) {
        callbacks.adjust_dialog_posture();
    }
    result.dialog_posture_adjusted = true;

    if (callbacks.refresh_combat_bars) {
        callbacks.refresh_combat_bars({0, 0xC69CU});
        callbacks.refresh_combat_bars({-1, 0xC79CU});
    }
    result.combat_bars_refreshed = true;
    return result;
}

std::uint16_t run_cyberware_menu(
    CyberwareMenuState& state,
    const CyberwareMenuCallbacks& callbacks) {
    if (callbacks.prepare_grid_variant_b) {
        callbacks.prepare_grid_variant_b();
    }
    if (callbacks.prepare_row_grid) {
        callbacks.prepare_row_grid();
    }

    state.rendered_entries = 0U;
    if (callbacks.render_title_text) {
        callbacks.render_title_text();
    }
    for (std::uint16_t bit = 0U; bit < 15U; ++bit) {
        if ((state.installed_flags & (std::uint16_t{1U} << bit)) == 0U) {
            continue;
        }
        if (callbacks.render_title_text) {
            callbacks.render_title_text();
        }
        ++state.rendered_entries;
    }
    if (callbacks.wait_for_controller_event) {
        callbacks.wait_for_controller_event();
    }
    if (callbacks.render_menu_row) {
        callbacks.render_menu_row();
    }
    return state.rendered_entries;
}

} // namespace shadowrun::ghidra_conversion
