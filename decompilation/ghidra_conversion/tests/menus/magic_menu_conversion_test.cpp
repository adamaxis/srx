#include "shadowrun_ghidra/menus/magic_menu_conversion.hpp"

#include <algorithm>
#include <vector>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    RunnerRecordFields shaman{};
    shaman.archetype_index = 1U;
    shaman.auxiliary[0] = 6U;
    shaman.magic_table_index = 3U;
    if (!expect(select_magic_menu_path(shaman) == MagicMenuPath::spell_panel)) {
        return 1;
    }

    shaman.auxiliary[0] = 0U;
    if (!expect(select_magic_menu_path(shaman) ==
                    MagicMenuPath::fallback_menu)) {
        return 2;
    }
    shaman.auxiliary[0] = 6U;
    shaman.archetype_index = 2U;
    if (!expect(select_magic_menu_path(shaman) ==
                    MagicMenuPath::fallback_menu)) {
        return 3;
    }

    MagicMenuLoopState menu_loop_state{{0U, 0U}, 14U};
    std::vector<std::uint16_t> menu_inputs{0x0C00U, 0x0300U, 0x2080U,
                                           0x8000U};
    std::size_t menu_input_index = 0U;
    std::vector<int> menu_loop_events;
    MagicMenuLoopCallbacks menu_loop_callbacks{};
    menu_loop_callbacks.prepare_magic_menu = [&]() { menu_loop_events.push_back(1); };
    menu_loop_callbacks.render_selection =
        [&](const MagicMenuSelection&) { menu_loop_events.push_back(2); };
    menu_loop_callbacks.advance_menu_sprite_overlay =
        [&]() { menu_loop_events.push_back(3); };
    menu_loop_callbacks.wait_for_vblank = [&]() { menu_loop_events.push_back(4); };
    menu_loop_callbacks.latch_controller_events = [&]() { menu_loop_events.push_back(5); };
    menu_loop_callbacks.sample_controller_input = [&]() { menu_loop_events.push_back(6); };
    menu_loop_callbacks.read_sampled_input = [&]() {
        menu_loop_events.push_back(7);
        return menu_inputs[menu_input_index++];
    };
    menu_loop_callbacks.run_equipment_selection =
        [&](const MagicMenuSelection&) { menu_loop_events.push_back(8); };
    menu_loop_callbacks.reset_cursor_animation_timer =
        [&]() { menu_loop_events.push_back(9); };
    const auto menu_exit = run_magic_menu_loop(menu_loop_state,
                                               menu_loop_callbacks);
    if (!expect(menu_exit == 0x8000U && menu_input_index == 4U &&
                menu_loop_state.selection.column == 6U &&
                menu_loop_state.selection.row == 1U &&
                menu_loop_events ==
                    std::vector<int>{1, 2, 3, 4, 5, 6, 7, 2, 9,
                                     3, 4, 5, 6, 7, 2, 9,
                                     3, 4, 5, 6, 7, 8, 2,
                                     3, 4, 5, 6, 7})) {
        return 37;
    }

    MagicMenuSelection selection{6U, 0U};
    selection = move_magic_selection_column(selection, 1, 7U);
    if (!expect(selection.column == 0U && selection.row == 0U)) {
        return 4;
    }
    selection = move_magic_selection_row(selection, 1, 7U);
    if (!expect(selection.column == 0U && selection.row == 1U)) {
        return 5;
    }
    selection = move_magic_selection_column({0U, 1U}, -1, 7U);
    if (!expect(selection.column == 0U && selection.row == 1U)) {
        return 6;
    }
    selection = move_magic_selection_row({1U, 1U}, 1, 7U);
    if (!expect(selection.column == 1U && selection.row == 0U)) {
        return 7;
    }
    selection = move_magic_selection_row({1U, 0U}, -1, 7U);
    if (!expect(selection.column == 1U && selection.row == 0U)) {
        return 8;
    }
    selection = move_magic_selection_row({0U, 0U}, 1, 6U);
    if (!expect(selection.column == 0U && selection.row == 0U)) {
        return 9;
    }

    std::vector<std::uint8_t> spell_table{
        0x00U, 0x02U, // two entries
        0x01U, 0x09U, // code 1, source table parameter 9
        0x03U, 0x04U, // code 3, source table parameter 4
    };
    std::vector<std::uint8_t> spell_records(4U * kMagicSpellRecordSize, 0U);
    spell_records[1U * kMagicSpellRecordSize + 0x25U] = 0x02U;
    spell_records[3U * kMagicSpellRecordSize + 0x25U] = 0xFFU;
    const auto available = select_magic_spell(
        spell_table, {0U, 0U}, spell_records);
    if (!expect(available.has_value() && available->table_entry_index == 0U &&
                available->spell_code == 1U &&
                available->table_parameter == 9U &&
                available->availability_byte == 2 &&
                available->availability == MagicSpellAvailability::available)) {
        return 10;
    }
    const auto unavailable = select_magic_spell(
        spell_table, {1U, 0U}, spell_records);
    if (!expect(unavailable.has_value() && unavailable->spell_code == 3U &&
                unavailable->availability_byte == -1 &&
                unavailable->availability ==
                    MagicSpellAvailability::unavailable &&
                !select_magic_spell(spell_table, {2U, 0U}, spell_records)
                     .has_value())) {
        return 11;
    }

    const std::vector<std::uint8_t> typed_spell_table{
        0x00U, 0x02U, // two entries
        0x02U, 0x07U, // Hellblast, source table parameter 7
        0x03U, 0x04U, // Invisibility, unavailable in the typed catalog
    };
    const auto typed_available = select_magic_spell(
        typed_spell_table, {0U, 0U}, us_magic_spell_catalog());
    const auto typed_unavailable = select_magic_spell(
        typed_spell_table, {1U, 0U}, us_magic_spell_catalog());
    if (!expect(typed_available.has_value() &&
                    typed_available->spell_code == 2U &&
                    typed_available->table_parameter == 7U &&
                    typed_available->availability_byte == 3 &&
                    typed_available->availability ==
                        MagicSpellAvailability::available &&
                    typed_unavailable.has_value() &&
                    typed_unavailable->spell_code == 3U &&
                    typed_unavailable->availability_byte == -1 &&
                    typed_unavailable->availability ==
                        MagicSpellAvailability::unavailable)) {
        return 15;
    }

    const auto typed_table = find_us_magic_spell_table(0U);
    const auto typed_table_selection = typed_table == nullptr
                                           ? std::nullopt
                                           : select_magic_spell(
                                                 *typed_table, {1U, 0U},
                                                 us_magic_spell_catalog());
    if (!expect(typed_table_selection.has_value() &&
                    typed_table_selection->table_entry_index == 1U &&
                    typed_table_selection->spell_code == 1U &&
                    typed_table_selection->table_parameter == 1U &&
                    typed_table_selection->availability_byte == 2 &&
                    typed_table_selection->availability ==
                        MagicSpellAvailability::available)) {
        return 16;
    }

    if (!expect(clamp_magic_force(1U, -1, 9U) == 1U &&
                clamp_magic_force(8U, 1, 9U) == 9U &&
                clamp_magic_force(3U, -1, 9U) == 2U)) {
        return 12;
    }
    MagicForceSelection force_state{1U};
    if (!expect(apply_magic_force_input(force_state, 0x0800U, 3U) ==
                    MagicForceLoopResult::adjusted &&
                force_state.force == 2U &&
                apply_magic_force_input(force_state, 0x0400U, 3U) ==
                    MagicForceLoopResult::adjusted &&
                force_state.force == 1U &&
                apply_magic_force_input(force_state, 0x0400U, 3U) ==
                    MagicForceLoopResult::adjusted &&
                force_state.force == 1U &&
                apply_magic_force_input(force_state, 0x0800U, 1U) ==
                    MagicForceLoopResult::adjusted &&
                force_state.force == 1U &&
                apply_magic_force_input(force_state, 0x2000U, 3U) ==
                    MagicForceLoopResult::waiting &&
                apply_magic_force_input(force_state, 0x0200U, 3U) ==
                    MagicForceLoopResult::finished &&
                apply_magic_force_input(force_state, 0U, 3U) ==
                    MagicForceLoopResult::waiting)) {
        return 13;
    }

    std::vector<int> loop_events;
    std::vector<std::uint16_t> loop_inputs{0x0800U, 0x2000U, 0x0200U};
    std::size_t loop_input_index = 0U;
    MagicForceSelectionLoopCallbacks loop_callbacks{};
    loop_callbacks.refresh_initial_display = [&]() { loop_events.push_back(1); };
    loop_callbacks.advance_menu_sprite_overlay = [&]() { loop_events.push_back(2); };
    loop_callbacks.wait_for_vblank = [&]() { loop_events.push_back(3); };
    loop_callbacks.latch_controller_events = [&]() { loop_events.push_back(4); };
    loop_callbacks.sample_controller_input = [&]() { loop_events.push_back(5); };
    loop_callbacks.read_sampled_input = [&]() {
        loop_events.push_back(6);
        return loop_inputs[loop_input_index++];
    };
    loop_callbacks.adjust_force = [&](const std::int16_t delta) {
        loop_events.push_back(delta > 0 ? 7 : 8);
    };
    loop_callbacks.refresh_final_display = [&]() { loop_events.push_back(9); };
    const auto loop_exit = run_magic_force_selection_loop(loop_callbacks);
    if (!expect(loop_events == std::vector<int>{1, 2, 3, 4, 5, 6, 7,
                                                 2, 3, 4, 5, 6,
                                                 2, 3, 4, 5, 6, 9} &&
                loop_exit == 0x0200U)) {
        return 14;
    }

    std::vector<int> force_display_events;
    MagicForceDisplayCallbacks force_display_callbacks{};
    force_display_callbacks.write_repeated_grid = [&]() {
        force_display_events.push_back(1);
    };
    force_display_callbacks.write_grid = [&]() {
        force_display_events.push_back(2);
    };
    refresh_magic_force_display(1U, 3U, force_display_callbacks);
    if (!expect(force_display_events == std::vector<int>{1, 2, 1, 1})) {
        return 15;
    }
    force_display_events.clear();
    refresh_magic_force_display(2U, 2U, force_display_callbacks);
    if (!expect(force_display_events == std::vector<int>{1, 1, 2, 1})) {
        return 16;
    }
    force_display_events.clear();
    refresh_magic_force_display(2U, 3U, force_display_callbacks);
    if (!expect(force_display_events == std::vector<int>{1, 1, 2, 1, 1})) {
        return 17;
    }

    MagicForceState selected_force{99U, 99U};
    RunnerRecordFields selected_record{};
    selected_record.equipped_inventory_slot = -1;
    selected_record.equipped_item_code = 2U;
    selected_record.magic_force = 3U;
    select_magic_force(selected_force, *available, selected_record);
    if (!expect(selected_force.maximum_force == 9U &&
                selected_force.current_force == 3U)) {
        return 18;
    }
    selected_record.equipped_item_code = 4U;
    select_magic_force(selected_force, *available, selected_record);
    if (!expect(selected_force.maximum_force == 9U &&
                selected_force.current_force == 9U)) {
        return 19;
    }
    selected_record.equipped_item_code = 2U;
    selected_record.equipped_inventory_slot = 0;
    select_magic_force(selected_force, *available, selected_record);
    if (!expect(selected_force.maximum_force == 9U &&
                selected_force.current_force == 9U)) {
        return 20;
    }

    int metadata_grid_writes = 0;
    prepare_magic_menu_grid_and_select_force(
        selected_force, *available, selected_record,
        {[&]() { ++metadata_grid_writes; }});
    if (!expect(metadata_grid_writes == 1 &&
                selected_force.maximum_force == 9U &&
                selected_force.current_force == 9U)) {
        return 32;
    }

    RunnerRecordFields equipment_record{};
    equipment_record.equipped_inventory_slot = 2;
    equipment_record.equipped_item_code = 7U;
    MagicForceState equipment_force{4U, 9U};
    std::vector<int> equipment_events;
    MagicEquipmentSelectionCallbacks equipment_callbacks{};
    equipment_callbacks.prepare_magic_action_details =
        [&](const MagicSpellDefinition& spell) {
            equipment_events.push_back(spell.spell_code == 1U ? 1 : 99);
        };
    equipment_callbacks.clear_entity_auxiliary_marker = [&]() {
        equipment_events.push_back(2);
    };
    equipment_callbacks.run_magic_force_selection_loop = [&]() {
        equipment_events.push_back(3);
        return static_cast<std::uint16_t>(0x4000U);
    };
    equipment_callbacks.run_runner_posture_adjustment = [&]() {
        equipment_events.push_back(4);
        return static_cast<std::uint16_t>(0U);
    };
    equipment_callbacks.upload_field_asset_setup = [&]() {
        equipment_events.push_back(5);
    };
    equipment_callbacks.draw_hud_panel_triplet = [&]() {
        equipment_events.push_back(6);
    };
    equipment_callbacks.advance_entity_behavior_meter = [&]() {
        equipment_events.push_back(7);
    };
    equipment_callbacks.advance_entity_asset_meter = [&]() {
        equipment_events.push_back(8);
    };
    const auto equipment_result = run_magic_equipment_selection(
        equipment_record, equipment_force, 1U, equipment_callbacks);
    if (!expect(!equipment_result.magic_spell_missing &&
                equipment_result.detail_preparation_called &&
                equipment_result.finalized &&
                equipment_record.equipped_item_code == 2U &&
                equipment_record.ammo_rounds == 4U &&
                equipment_record.equipped_inventory_slot == -1 &&
                equipment_record.magic_force == 4U &&
                equipment_events == std::vector<int>{1, 2, 3, 5, 6, 7, 8})) {
        return 30;
    }

    RunnerRecordFields missing_spell_record{};
    MagicForceState missing_spell_force{};
    const auto missing_spell = run_magic_equipment_selection(
        missing_spell_record, missing_spell_force, 13U);
    if (!expect(missing_spell.magic_spell_missing &&
                !missing_spell.detail_preparation_called &&
                !missing_spell.finalized)) {
        return 33;
    }

    RunnerRecordFields posture_record{};
    posture_record.equipped_inventory_slot = -1;
    MagicForceState posture_force{2U, 6U};
    std::vector<int> posture_events;
    std::vector<std::uint16_t> force_exits{0U, 0x1000U};
    std::size_t force_exit_index = 0U;
    MagicEquipmentSelectionCallbacks posture_callbacks{};
    posture_callbacks.prepare_magic_action_details =
        [&](const MagicSpellDefinition& spell) {
            posture_events.push_back(spell.spell_code == 3U ? 1 : 99);
        };
    posture_callbacks.run_magic_force_selection_loop = [&]() {
        posture_events.push_back(2);
        return force_exits[force_exit_index++];
    };
    posture_callbacks.run_runner_posture_adjustment = [&]() {
        posture_events.push_back(3);
        return static_cast<std::uint16_t>(0U);
    };
    posture_callbacks.upload_field_asset_setup = [&]() {
        posture_events.push_back(4);
    };
    posture_callbacks.draw_hud_panel_triplet = [&]() { posture_events.push_back(5); };
    posture_callbacks.advance_entity_behavior_meter = [&]() { posture_events.push_back(6); };
    posture_callbacks.advance_entity_asset_meter = [&]() { posture_events.push_back(7); };
    run_magic_equipment_selection(posture_record, posture_force, 3U,
                                  posture_callbacks);
    if (!expect(posture_record.equipped_item_code == 4U &&
                posture_record.equipped_inventory_slot == -1 &&
                posture_record.magic_force == 2U &&
                posture_events == std::vector<int>{1, 2, 3, 2, 4, 5, 6, 7})) {
        return 31;
    }

    std::vector<int> detail_events;
    MagicActionDetailCallbacks detail_callbacks{};
    detail_callbacks.prepare_menu_grid_then_service = [&]() {
        detail_events.push_back(1);
    };
    detail_callbacks.render_action_text =
        [&](const MagicActionTextRegion region) {
            detail_events.push_back(static_cast<int>(region) + 2);
        };
    detail_callbacks.adjust_runner_posture = [&]() {
        detail_events.push_back(5);
    };
    detail_callbacks.render_spell_availability =
        [&](const MagicSpellDefinition& spell) {
            detail_events.push_back(spell.spell_code == 0U ? 6 : 99);
        };
    detail_callbacks.build_grouped_decimal_glyphs = [&]() {
        detail_events.push_back(7);
    };
    prepare_magic_action_details(*find_us_magic_spell(0U), detail_callbacks);
    if (!expect(detail_events == std::vector<int>{1, 2, 3, 4, 5, 6, 7})) {
        return 21;
    }

    std::vector<int> selection_display_events;
    std::uint8_t transferred_spell = 0U;
    std::uint16_t transferred_mask = 0U;
    MagicMenuSelectionDisplayCallbacks selection_display_callbacks{};
    selection_display_callbacks.prepare_menu_grid_then_service = [&]() {
        selection_display_events.push_back(1);
    };
    selection_display_callbacks.refresh_force_display = [&]() {
        selection_display_events.push_back(2);
    };
    selection_display_callbacks.transfer_spell_glyph_triple =
        [&](const std::uint8_t spell_code, const std::uint16_t tile_mask) {
            selection_display_events.push_back(3);
            transferred_spell = spell_code;
            transferred_mask = tile_mask;
        };
    selection_display_callbacks.render_spell_record_text =
        [&](const MagicSpellDefinition& spell,
            const MagicSpellRecordTextRegion region) {
            if (spell.spell_code != transferred_spell &&
                (region == MagicSpellRecordTextRegion::spell_name ||
                 region == MagicSpellRecordTextRegion::spell_description)) {
                // The transfer callback runs immediately before these text
                // callbacks, so this catches a stale/detached spell row.
                selection_display_events.push_back(99);
                return;
            }
            selection_display_events.push_back(
                region == MagicSpellRecordTextRegion::spell_name ? 4 : 5);
        };
    selection_display_callbacks.render_action_text =
        [&](const MagicActionTextRegion region) {
            selection_display_events.push_back(
                region == MagicActionTextRegion::success_label ? 6 : 7);
        };
    selection_display_callbacks.adjust_runner_posture = [&]() {
        selection_display_events.push_back(8);
    };
    selection_display_callbacks.render_spell_availability =
        [&](const MagicSpellDefinition& spell) {
            selection_display_events.push_back(
                spell.spell_code == transferred_spell ? 9 : 99);
        };
    selection_display_callbacks.build_grouped_decimal_glyphs = [&]() {
        selection_display_events.push_back(10);
    };
    render_magic_menu_selection_display(*find_us_magic_spell(5U),
                                         selection_display_callbacks);
    if (!expect(selection_display_events ==
                    std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10} &&
                transferred_spell == 5U && transferred_mask == 0x2000U)) {
        return 22;
    }
    selection_display_events.clear();
    render_magic_menu_selection_display(*find_us_magic_spell(6U),
                                         selection_display_callbacks);
    if (!expect(selection_display_events ==
                    std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10} &&
                transferred_spell == 6U && transferred_mask == 0x6000U)) {
        return 23;
    }

    MagicForceState force_state_values{3U, 5U};
    RunnerRecordFields force_record{};
    std::vector<int> adjustment_events;
    MagicForceAdjustmentCallbacks adjustment_callbacks{};
    adjustment_callbacks.build_grouped_decimal_glyphs = [&]() {
        adjustment_events.push_back(1);
    };
    adjustment_callbacks.display.write_repeated_grid = [&]() {
        adjustment_events.push_back(2);
    };
    adjustment_callbacks.display.write_grid = [&]() {
        adjustment_events.push_back(3);
    };
    adjustment_callbacks.write_repeated_grid = [&]() {
        adjustment_events.push_back(4);
    };
    adjustment_callbacks.refresh_magic_menu_success_or_defense_values = [&]() {
        adjustment_events.push_back(5);
        force_state_values.current_force = 99U;
        force_state_values.maximum_force = 100U;
    };
    adjustment_callbacks.write_repeated_menu_grid = [&]() {
        adjustment_events.push_back(6);
    };
    adjustment_callbacks.refresh_magic_action_effect_value = [&]() {
        adjustment_events.push_back(7);
    };
    adjust_magic_force(force_record, force_state_values, 1,
                       adjustment_callbacks);
    if (!expect(force_state_values.current_force == 4U &&
                force_state_values.maximum_force == 100U &&
                force_record.magic_force == 4U &&
                adjustment_events == std::vector<int>{1, 2, 2, 3, 2, 2, 4,
                                                      5, 6, 7})) {
        return 24;
    }

    std::vector<int> availability_events;
    MagicSpellAvailabilityCallbacks availability_callbacks{};
    availability_callbacks.write_repeated_grid = [&]() {
        availability_events.push_back(1);
    };
    availability_callbacks.render_spell_text =
        [&](const MagicSpellDefinition& spell) {
            if (spell.spell_code != 0U || spell.display_name != "Flame Dart") {
                availability_events.push_back(99);
                return;
            }
            availability_events.push_back(2);
        };
    availability_callbacks.write_grid = [&]() {
        availability_events.push_back(3);
    };
    render_magic_spell_availability(*find_us_magic_spell(3U),
                                    availability_callbacks);
    if (!expect(availability_events == std::vector<int>{1})) {
        return 25;
    }
    availability_events.clear();
    render_magic_spell_availability(*find_us_magic_spell(0U),
                                    availability_callbacks);
    if (!expect(availability_events == std::vector<int>{2, 3})) {
        return 26;
    }

    std::vector<int> grid_events;
    MagicGridControlCallbacks grid_callbacks{};
    grid_callbacks.write_grid = [&]() { grid_events.push_back(1); };
    grid_callbacks.set_display_command =
        [&](const std::uint16_t command) {
            grid_events.push_back(static_cast<int>(command));
        };
    clear_magic_menu_grids(false, grid_callbacks);
    if (!expect(grid_events.size() == 10U && grid_events[9] == 0x8174 &&
                std::count(grid_events.begin(), grid_events.end(), 1) == 9)) {
        return 27;
    }
    grid_events.clear();
    clear_magic_menu_grids(true, grid_callbacks);
    if (!expect(grid_events.size() == 11U && grid_events[10] == 0x8174 &&
                std::count(grid_events.begin(), grid_events.end(), 1) == 10)) {
        return 28;
    }
    grid_events.clear();
    refresh_menu_grid_quadruple(grid_callbacks);
    if (!expect(grid_events ==
                std::vector<int>{0x8134, 1, 1, 1, 1, 0x8174})) {
        return 29;
    }

    EntityActionRecord fallback_actor{};
    fallback_actor.attribute_77 = 5U;
    fallback_actor.attribute_7c = 1U;
    std::array<EntityActionRecord, 3> idle_party{};
    const auto physical = prepare_magic_fallback_defense_strip(
        idle_party, fallback_actor, false);
    if (!expect(physical.strip == MagicMenuValueStrip::defense_physical &&
                physical.value == 5U && physical.row == 4U &&
                physical.destination_tile_base ==
                    kMagicFallbackPhysicalDestination &&
                physical.palette_flags == kMagicFallbackDefensePalette &&
                physical.glyph_source_base == kMagicFallbackDefenseGlyphBase &&
                physical.source_row_stride == 0x1AU &&
                physical.tile_count == 13U)) {
        return 38;
    }
    const auto mana = prepare_magic_fallback_defense_strip(
        idle_party, fallback_actor, true);
    if (!expect(mana.strip == MagicMenuValueStrip::defense_mana &&
                mana.value == 1U && mana.row == 0U &&
                mana.destination_tile_base == kMagicFallbackManaDestination)) {
        return 39;
    }
    fallback_actor.attribute_77 = 0U;
    const auto empty = prepare_magic_fallback_defense_strip(
        idle_party, fallback_actor, false);
    if (!expect(empty.value == 0U && empty.row == 0U)) {
        return 40;
    }

    const auto* hellblast = find_us_magic_spell(2U);
    if (!expect(hellblast != nullptr)) {
        return 41;
    }
    MagicActionEffectValueInput effect_input{};
    effect_input.current_force = 1U;
    effect_input.spell = *hellblast;
    const auto effect = refresh_magic_action_effect_value(effect_input);
    if (!expect(effect.residual_valid && effect.residual_effect == 10 &&
                effect.damage_grid.has_value() &&
                effect.damage_grid->effect_row == 9U &&
                effect.damage_grid->tile_count == 7U &&
                effect.damage_grid->source_row_stride == 0x0EU)) {
        return 42;
    }

    return 0;
}
