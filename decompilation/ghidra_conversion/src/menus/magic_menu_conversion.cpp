#include "shadowrun_ghidra/menus/magic_menu_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {
namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

std::optional<MagicSpellSelection> make_typed_magic_spell_selection(
    const std::uint16_t entry_index, const std::uint8_t spell_code,
    const std::uint8_t table_parameter,
    const UsMagicSpellCatalog& spell_catalog) {
    const auto spell = std::find_if(
        spell_catalog.begin(), spell_catalog.end(),
        [spell_code](const MagicSpellDefinition& candidate) {
            return candidate.spell_code == spell_code;
        });
    if (spell == spell_catalog.end()) {
        return std::nullopt;
    }

    MagicSpellSelection result{};
    result.table_entry_index = entry_index;
    result.spell_code = spell_code;
    result.table_parameter = table_parameter;
    result.availability_byte = spell->availability_byte;
    result.availability = spell->availability_byte >= 0
                              ? MagicSpellAvailability::available
                              : MagicSpellAvailability::unavailable;
    return result;
}

std::int16_t maximum_magic_column(const std::uint8_t row,
                                  const std::uint16_t first_entry_count) {
    auto maximum = static_cast<std::int16_t>(first_entry_count);
    if (row == 0U) {
        if (maximum >= 7) {
            maximum = 6;
        }
    } else {
        maximum = static_cast<std::int16_t>(maximum - 7);
    }
    return maximum;
}

} // namespace

MagicMenuPath select_magic_menu_path(const RunnerRecordFields& record) {
    if (record.archetype_index < 2U && record.auxiliary[0] != 0U) {
        return MagicMenuPath::spell_panel;
    }
    return MagicMenuPath::fallback_menu;
}

std::uint16_t run_magic_menu_loop(MagicMenuLoopState& state,
                                  const MagicMenuLoopCallbacks& callbacks) {
    // ROM 0xFBA6 performs its one-time resource/panel setup before the
    // controller loop and renders the initial selected spell immediately.
    call_once(callbacks.prepare_magic_menu);
    if (callbacks.render_selection) {
        callbacks.render_selection(state.selection);
    }

    for (;;) {
        call_once(callbacks.advance_menu_sprite_overlay);
        call_once(callbacks.wait_for_vblank);
        call_once(callbacks.latch_controller_events);
        call_once(callbacks.sample_controller_input);
        const auto input = callbacks.read_sampled_input
                               ? callbacks.read_sampled_input()
                               : 0U;

        // 0xFD02 checks column-left before column-right. Preserve that
        // priority if both source edges arrive in one sampled word.
        if ((input & 0x0400U) != 0U) {
            state.selection = move_magic_selection_column(
                state.selection, -1, state.first_entry_count);
            if (callbacks.render_selection) {
                callbacks.render_selection(state.selection);
            }
            call_once(callbacks.reset_cursor_animation_timer);
            continue;
        }
        if ((input & 0x0800U) != 0U) {
            state.selection = move_magic_selection_column(
                state.selection, 1, state.first_entry_count);
            if (callbacks.render_selection) {
                callbacks.render_selection(state.selection);
            }
            call_once(callbacks.reset_cursor_animation_timer);
            continue;
        }

        // 0xFD32 checks row-up before row-down, with the same redraw/reset
        // sequence as horizontal navigation.
        if ((input & 0x0100U) != 0U) {
            state.selection = move_magic_selection_row(
                state.selection, -1, state.first_entry_count);
            if (callbacks.render_selection) {
                callbacks.render_selection(state.selection);
            }
            call_once(callbacks.reset_cursor_animation_timer);
            continue;
        }
        if ((input & 0x0200U) != 0U) {
            state.selection = move_magic_selection_row(
                state.selection, 1, state.first_entry_count);
            if (callbacks.render_selection) {
                callbacks.render_selection(state.selection);
            }
            call_once(callbacks.reset_cursor_animation_timer);
            continue;
        }

        // Action edges are checked before the source bit-7 exit condition.
        if ((input & (0x2000U | 0x1000U | 0x4000U)) != 0U) {
            if (callbacks.run_equipment_selection) {
                callbacks.run_equipment_selection(state.selection);
            }
            if (callbacks.render_selection) {
                callbacks.render_selection(state.selection);
            }
            continue;
        }
        if ((input & 0x0080U) != 0U) {
            return input;
        }
    }
}

MagicActionEffectValueResult refresh_magic_action_effect_value(
    const MagicActionEffectValueInput& input,
    const MagicActionEffectValueCallbacks& callbacks) {
    MagicActionEffectValueResult result{};
    // ROM 0x105E6 enters 0x10660 with D4=current force and D7=action code.
    const EntityActionCatalogRecord action{
        input.spell.difficulty_modifier, input.spell.base_effect};
    const auto residual = calculate_action_residual(
        input.spell.spell_code, input.current_force,
        calculate_action_effect_roll_count(input.entity), action);
    result.residual_valid = residual.valid;
    result.residual_effect = residual.residual_effect;
    if (!residual.valid || residual.residual_effect <= 0) {
        const MagicActionEffectNoneValuePlan plan{};
        result.none_damage_value = plan;
        if (callbacks.render_none_damage_value) {
            callbacks.render_none_damage_value(plan);
            result.none_damage_value_rendered = true;
        }
        return result;
    }

    MagicActionEffectDamageGridPlan plan{};
    plan.effect_row = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(residual.residual_effect) - 1U);
    result.damage_grid = plan;
    if (callbacks.write_damage_grid) {
        callbacks.write_damage_grid(plan);
        result.damage_grid_written = true;
    }
    return result;
}

MagicMenuValueRefreshResult refresh_magic_menu_success_or_defense_values(
    const MagicMenuValueRefreshInput& input,
    const MagicMenuValueRefreshCallbacks& callbacks) {
    MagicMenuValueRefreshResult result{};
    if (!input.refresh_defense_strips) {
        // ROM 0x1055E/D0=0: D7=0 selects the posture-reduction component
        // used for the magic action's Success strip.
        const auto item18 = calculate_item18_skill_value(input.active_entity);
        auto value = select_posture_adjusted_value(
            item18, input.active_entity.posture, false);
        value = std::min<std::uint16_t>(value, input.action_cap);
        value = static_cast<std::uint16_t>(value + input.current_force);
        if (callbacks.resolve_menu_metric) {
            const auto resolved = callbacks.resolve_menu_metric(value);
            if (resolved.has_value()) {
                value = *resolved;
                result.menu_metric_available = true;
            }
        }
        result.success_value = value;
    } else {
        // ROM 0x1055E/D0!=0 enters 0x1047E before adding +0x77/+0x7C for
        // the physical and mana Defense strips.
        call_once(callbacks.refresh_ready_entity_animations);
        const auto ready_bonus = calculate_ready_entity_action_bonus(
            input.ready_entities);
        result.defense_physical_value = static_cast<std::uint8_t>(
            ready_bonus + input.attribute_77);
        result.defense_mana_value = static_cast<std::uint8_t>(
            ready_bonus + input.attribute_7c);
    }

    const auto publish_grid = [&](const MagicMenuValueStrip strip,
                                  const std::uint16_t value,
                                  const std::uint16_t destination) {
        const auto row = static_cast<std::uint16_t>(
            value == 0U ? 0U : static_cast<std::uint16_t>(value - 1U));
        const MagicMenuValueGridPlan plan{strip, value, row, destination};
        result.grid_requests.push_back(plan);
        if (callbacks.write_grid) {
            callbacks.write_grid(plan);
        }
    };

    if (!input.refresh_defense_strips) {
        publish_grid(MagicMenuValueStrip::success, result.success_value,
                     0xC588U);
    } else {
        publish_grid(MagicMenuValueStrip::defense_physical,
                     result.defense_physical_value, 0xC808U);
        if (result.defense_mana_value != 0U) {
            publish_grid(MagicMenuValueStrip::defense_mana,
                         result.defense_mana_value, 0xC908U);
        }
    }
    return result;
}

MagicMenuSelection move_magic_selection_column(
    MagicMenuSelection selection, const std::int8_t delta,
    const std::uint16_t first_entry_count) {
    const auto maximum = maximum_magic_column(selection.row,
                                              first_entry_count);
    const auto next = static_cast<std::int16_t>(selection.column) + delta;
    if (next < 0) {
        selection.column = static_cast<std::uint8_t>(
            std::max<std::int16_t>(0, maximum));
    } else if (next <= maximum) {
        selection.column = static_cast<std::uint8_t>(next);
    } else {
        selection.column = 0U;
    }
    return selection;
}

MagicMenuSelection move_magic_selection_row(
    MagicMenuSelection selection, const std::int8_t delta,
    const std::uint16_t first_entry_count) {
    if (static_cast<std::int16_t>(first_entry_count) - 7 < 0) {
        return selection;
    }

    const auto next = static_cast<std::int16_t>(selection.row) + delta;
    if (next < 0) {
        selection.row = 1U;
    } else if (next == 2) {
        selection.row = 0U;
    } else {
        selection.row = static_cast<std::uint8_t>(next);
    }

    if (maximum_magic_column(selection.row, first_entry_count) <
        static_cast<std::int16_t>(selection.column)) {
        selection.row = 0U;
    }
    return selection;
}

std::optional<std::int8_t> read_magic_spell_availability(
    const std::vector<std::uint8_t>& spell_records,
    const std::uint8_t spell_code) {
    const auto record_offset = static_cast<std::size_t>(spell_code) *
                               kMagicSpellRecordSize;
    constexpr std::size_t kAvailabilityOffset = 0x25U;
    if (record_offset + kAvailabilityOffset >= spell_records.size()) {
        return std::nullopt;
    }
    return static_cast<std::int8_t>(spell_records[record_offset +
                                                  kAvailabilityOffset]);
}

std::optional<MagicSpellSelection> select_magic_spell(
    const std::vector<std::uint8_t>& spell_table,
    const MagicMenuSelection selection,
    const std::vector<std::uint8_t>& spell_records) {
    if (spell_table.size() < 2U || selection.row > 1U ||
        selection.column >= 7U) {
        return std::nullopt;
    }
    const auto count = static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(spell_table[0]) << 8U) |
        spell_table[1]);
    const auto entry_index = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(selection.row) * 7U + selection.column);
    if (entry_index >= count) {
        return std::nullopt;
    }
    const auto entry_offset = static_cast<std::size_t>(2U) +
                              static_cast<std::size_t>(entry_index) * 2U;
    if (entry_offset + 1U >= spell_table.size()) {
        return std::nullopt;
    }

    const auto availability =
        read_magic_spell_availability(spell_records, spell_table[entry_offset]);
    if (!availability.has_value()) {
        return std::nullopt;
    }
    MagicSpellSelection result{};
    result.table_entry_index = entry_index;
    result.spell_code = spell_table[entry_offset];
    result.table_parameter = spell_table[entry_offset + 1U];
    result.availability_byte = *availability;
    result.availability = *availability >= 0
                              ? MagicSpellAvailability::available
                              : MagicSpellAvailability::unavailable;
    return result;
}

std::optional<MagicSpellSelection> select_magic_spell(
    const std::vector<std::uint8_t>& spell_table,
    const MagicMenuSelection selection,
    const UsMagicSpellCatalog& spell_catalog) {
    if (spell_table.size() < 2U || selection.row > 1U ||
        selection.column >= 7U) {
        return std::nullopt;
    }
    const auto count = static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(spell_table[0]) << 8U) |
        spell_table[1]);
    const auto entry_index = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(selection.row) * 7U + selection.column);
    if (entry_index >= count) {
        return std::nullopt;
    }
    const auto entry_offset = static_cast<std::size_t>(2U) +
                              static_cast<std::size_t>(entry_index) * 2U;
    if (entry_offset + 1U >= spell_table.size()) {
        return std::nullopt;
    }

    return make_typed_magic_spell_selection(
        entry_index, spell_table[entry_offset], spell_table[entry_offset + 1U],
        spell_catalog);
}

std::optional<MagicSpellSelection> select_magic_spell(
    const MagicSpellTable& spell_table,
    const MagicMenuSelection selection,
    const UsMagicSpellCatalog& spell_catalog) {
    if (selection.row > 1U || selection.column >= 7U) {
        return std::nullopt;
    }
    const auto entry_index = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(selection.row) * 7U + selection.column);
    if (entry_index >= spell_table.entry_count ||
        entry_index >= kMagicSpellTableEntryCapacity) {
        return std::nullopt;
    }
    const auto& entry = spell_table.entries[entry_index];
    return make_typed_magic_spell_selection(
        entry_index, entry.spell_code, entry.force_limit, spell_catalog);
}

std::uint8_t clamp_magic_force(const std::uint8_t current,
                               const std::int8_t delta,
                               const std::uint8_t table_parameter) {
    auto next = static_cast<std::int16_t>(current) + delta;
    if (next > static_cast<std::int16_t>(table_parameter)) {
        next = table_parameter;
    }
    if (next < 1) {
        next = 1;
    }
    return static_cast<std::uint8_t>(next);
}

MagicForceLoopResult apply_magic_force_input(
    MagicForceSelection& state, const std::uint16_t input,
    const std::uint8_t table_parameter) {
    constexpr std::uint16_t kRaiseForce = 0x0800U;
    constexpr std::uint16_t kLowerForce = 0x0400U;
    // ROM 0x10016 exits on bits 9, E, or C. Bit D is the explicit loop-back
    // edge at 0x10092 and must not be folded into the finish mask.
    constexpr std::uint16_t kFinishInput = 0x0200U | 0x4000U | 0x1000U;
    if ((input & kRaiseForce) != 0U) {
        state.force = clamp_magic_force(state.force, 1, table_parameter);
        return MagicForceLoopResult::adjusted;
    }
    if ((input & kLowerForce) != 0U) {
        state.force = clamp_magic_force(state.force, -1, table_parameter);
        return MagicForceLoopResult::adjusted;
    }
    if ((input & kFinishInput) != 0U) {
        return MagicForceLoopResult::finished;
    }
    return MagicForceLoopResult::waiting;
}

std::uint16_t run_magic_force_selection_loop(
    const MagicForceSelectionLoopCallbacks& callbacks) {
    // ROM 0x10016 enters with the C85A8 force-display variant, then redraws
    // with C85AC only after bit 9, E, or C returns from the frame loop.
    call_once(callbacks.refresh_initial_display);
    for (;;) {
        call_once(callbacks.advance_menu_sprite_overlay);
        call_once(callbacks.wait_for_vblank);
        call_once(callbacks.latch_controller_events);
        call_once(callbacks.sample_controller_input);

        const auto input = callbacks.read_sampled_input
                               ? callbacks.read_sampled_input()
                               : 0U;
        if ((input & 0x0800U) != 0U) {
            // Source tests B before A when both edges are present.
            if (callbacks.adjust_force) {
                callbacks.adjust_force(1);
            }
            continue;
        }
        if ((input & 0x0400U) != 0U) {
            if (callbacks.adjust_force) {
                callbacks.adjust_force(-1);
            }
            continue;
        }
        if ((input & (0x0200U | 0x4000U | 0x1000U)) != 0U) {
            call_once(callbacks.refresh_final_display);
            return input;
        }
        // Bit D and an empty input word both follow the source loop-back edge.
    }
}

void run_magic_equipment_selection(
    RunnerRecordFields& record, MagicForceState& force_state,
    const std::uint8_t spell_code,
    const MagicEquipmentSelectionCallbacks& callbacks) {
    // ROM 0xFF5A enters through the detail panel before inspecting the
    // currently equipped item. The detail callback is where source 0x102D4
    // establishes the current force shown to the player.
    call_once(callbacks.prepare_magic_action_details);

    const bool has_active_item = record.equipped_inventory_slot >= 0 &&
                                 record.equipped_item_code != 0U;
    if (has_active_item) {
        call_once(callbacks.clear_entity_auxiliary_marker);
        record.ammo_rounds = static_cast<std::uint8_t>(force_state.current_force);
    }
    record.equipped_item_code = static_cast<std::uint8_t>(
        static_cast<unsigned>(spell_code) + 1U);

    // The source checks D, C, then E after force selection. An E edge enters
    // posture adjustment only when it was absent from the force-loop result.
    for (;;) {
        const auto force_input = callbacks.run_magic_force_selection_loop
                                     ? callbacks.run_magic_force_selection_loop()
                                     : 0U;
        if ((force_input & 0x2000U) != 0U ||
            (force_input & 0x1000U) != 0U ||
            (force_input & 0x4000U) != 0U) {
            break;
        }

        const auto posture_input = callbacks.run_runner_posture_adjustment
                                       ? callbacks.run_runner_posture_adjustment()
                                       : 0U;
        if ((posture_input & 0x2000U) != 0U ||
            (posture_input & 0x1000U) != 0U) {
            break;
        }
        // Only the absence of E after posture returns to the force selector;
        // D/C already exited above, and the source falls through on E.
        if ((posture_input & 0x4000U) != 0U) {
            break;
        }
    }

    // 0xFFB6 finalizes the selected spell and publishes the field-facing
    // meters before storing the selected force in record +0x9D.
    record.equipped_inventory_slot = -1;
    call_once(callbacks.upload_field_asset_setup);
    call_once(callbacks.draw_hud_panel_triplet);
    call_once(callbacks.advance_entity_behavior_meter);
    call_once(callbacks.advance_entity_asset_meter);
    record.magic_force = static_cast<std::uint8_t>(force_state.current_force);
}

void refresh_magic_force_display(
    const std::uint16_t current_force, const std::uint16_t maximum_force,
    const MagicForceDisplayCallbacks& callbacks) {
    // ROM 0x10172 calls 0x10156 before the conditional force marker.
    call_once(callbacks.write_repeated_grid);
    if (current_force >= 2U) {
        call_once(callbacks.write_repeated_grid);
    }
    call_once(callbacks.write_grid);
    if (maximum_force != current_force) {
        call_once(callbacks.write_repeated_grid);
    }
    call_once(callbacks.write_repeated_grid);
}

void select_magic_force(MagicForceState& state,
                        const MagicSpellSelection& selection,
                        const RunnerRecordFields& record) {
    // 0x101FE stores the low byte of the selected FFF9FA word in FFF098.
    state.maximum_force = selection.table_parameter;

    // The source compares the selected high byte plus one with record +0x57
    // only on the negative +0x56 (no inventory slot) path. The byte compare
    // deliberately wraps, matching the source's low-byte comparison.
    const auto selected_item_code = static_cast<std::uint8_t>(
        static_cast<unsigned>(selection.spell_code) + 1U);
    if (record.equipped_inventory_slot < 0 &&
        selected_item_code == record.equipped_item_code) {
        state.current_force = record.magic_force;
        return;
    }
    state.current_force = state.maximum_force;
}

void prepare_magic_menu_grid_and_select_force(
    MagicForceState& state, const MagicSpellSelection& selection,
    const RunnerRecordFields& record,
    const MagicMenuGridPreparationCallbacks& callbacks) {
    // ROM 0x10260 submits the metadata grid before 0x101FE mutates the
    // selected current/max force pair.
    call_once(callbacks.write_menu_repeated_grid_with_metadata);
    select_magic_force(state, selection, record);
}

void adjust_magic_force(
    RunnerRecordFields& record, MagicForceState& state,
    const std::int16_t delta, const MagicForceAdjustmentCallbacks& callbacks) {
    // ROM 0x100A8 performs a word add, clamps to the selected maximum, and
    // then replaces zero with one before publishing the byte field.
    auto next = static_cast<std::int32_t>(state.current_force) + delta;
    next = std::min(next, static_cast<std::int32_t>(state.maximum_force));
    next = std::max(next, 1);
    state.current_force = static_cast<std::uint16_t>(next);
    record.magic_force = static_cast<std::uint8_t>(state.current_force);

    call_once(callbacks.build_grouped_decimal_glyphs);
    refresh_magic_force_display(state.current_force, state.maximum_force,
                                callbacks.display);
    call_once(callbacks.write_repeated_grid);

    // ROM 0x100A8 saves only FFF096 around 0x1055E.  The equipment refresh
    // may replace the selected maximum at FFF098; that replacement is part
    // of the source state and must not be silently rolled back here.
    const auto saved_current = state.current_force;
    call_once(callbacks.refresh_magic_menu_success_or_defense_values);
    state.current_force = saved_current;

    call_once(callbacks.write_repeated_menu_grid);
    call_once(callbacks.refresh_magic_action_effect_value);
}

void prepare_magic_action_details(
    const MagicActionDetailCallbacks& callbacks) {
    // ROM 0x102D4 preserves three distinct text-render calls between the
    // grid preparation and posture/availability services.
    call_once(callbacks.prepare_menu_grid_then_service);
    if (callbacks.render_action_text) {
        callbacks.render_action_text(
            MagicActionTextRegion::set_force_and_posture_heading);
        callbacks.render_action_text(MagicActionTextRegion::success_label);
        callbacks.render_action_text(MagicActionTextRegion::force_label);
    }
    call_once(callbacks.adjust_runner_posture);
    call_once(callbacks.render_spell_availability);
    call_once(callbacks.build_grouped_decimal_glyphs);
}

void render_magic_menu_selection_display(
    const std::uint8_t spell_code,
    const MagicMenuSelectionDisplayCallbacks& callbacks) {
    // ROM 0x10276 sets D6 to the source spell-art tile class before the
    // record glyph triple. Codes below six use 0x2000; the remaining spell
    // rows use 0x6000.
    call_once(callbacks.prepare_menu_grid_then_service);
    call_once(callbacks.refresh_force_display);
    if (callbacks.transfer_spell_glyph_triple) {
        callbacks.transfer_spell_glyph_triple(
            spell_code, spell_code < 6U ? 0x2000U : 0x6000U);
    }
    if (callbacks.render_spell_record_text) {
        callbacks.render_spell_record_text(
            spell_code, MagicSpellRecordTextRegion::spell_name);
        callbacks.render_spell_record_text(
            spell_code, MagicSpellRecordTextRegion::spell_description);
    }

    // 0x102D4 begins at the shared tail reached by the branch at 0x102D0.
    if (callbacks.render_action_text) {
        callbacks.render_action_text(MagicActionTextRegion::success_label);
        callbacks.render_action_text(MagicActionTextRegion::force_label);
    }
    call_once(callbacks.adjust_runner_posture);
    call_once(callbacks.render_spell_availability);
    call_once(callbacks.build_grouped_decimal_glyphs);
}

void render_magic_spell_availability(
    const std::int8_t availability_byte,
    const MagicSpellAvailabilityCallbacks& callbacks) {
    // ROM 0x1088C branches on the signed +0x25 spell-record byte.
    if (availability_byte < 0) {
        call_once(callbacks.write_repeated_grid);
        return;
    }
    call_once(callbacks.render_spell_text);
    call_once(callbacks.write_grid);
}

void clear_magic_menu_grids(
    const bool include_extra_grid,
    const MagicGridControlCallbacks& callbacks) {
    // ROM 0x1098E writes nine grids unconditionally and a tenth only when
    // FFF0AF is nonzero.
    for (int index = 0; index < 9; ++index) {
        call_once(callbacks.write_grid);
    }
    if (include_extra_grid) {
        call_once(callbacks.write_grid);
    }
    if (callbacks.set_display_command) {
        callbacks.set_display_command(0x8174U);
    }
}

void refresh_menu_grid_quadruple(
    const MagicGridControlCallbacks& callbacks) {
    // ROM 0x10A82 brackets four grid writes with the two source display
    // commands; these are typed presentation callbacks on the host.
    if (callbacks.set_display_command) {
        callbacks.set_display_command(0x8134U);
    }
    for (int index = 0; index < 4; ++index) {
        call_once(callbacks.write_grid);
    }
    if (callbacks.set_display_command) {
        callbacks.set_display_command(0x8174U);
    }
}

} // namespace shadowrun::ghidra_conversion
