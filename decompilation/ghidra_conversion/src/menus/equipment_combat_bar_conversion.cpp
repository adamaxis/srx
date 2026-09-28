#include "shadowrun_ghidra/menus/equipment_combat_bar_conversion.hpp"

#include <algorithm>
#include <cstddef>

namespace shadowrun::ghidra_conversion {
namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

} // namespace

std::uint16_t calculate_equipment_command_average(
    const EquipmentCombatInput& input) {
    // Keep one source-faithful 0x55ED6 implementation. This menu-facing
    // overload only decodes its presentation input into the same typed
    // entity record consumed by the late action/effect routines.
    EntityActionRecord entity{};
    entity.attribute_78 = input.attributes[1];
    entity.attribute_7b = input.attributes[4];
    entity.attribute_7c = input.attributes[5];
    entity.item_codes = input.inventory_item_codes;
    entity.item_phases = input.inventory_phase_bytes;
    return calculate_equipment_command_average(entity);
}

namespace {

std::uint16_t posture_value(const std::uint16_t base,
                            const std::uint8_t posture,
                            const bool defense) {
    const auto penalty = static_cast<unsigned>(base) * posture * 0x19U / 0x64U;
    if (defense) {
        return static_cast<std::uint16_t>(
            base - std::min<unsigned>(base, penalty));
    }
    return static_cast<std::uint16_t>(penalty);
}

} // namespace

EquipmentActionSkillMergeResult merge_equipment_action_skill_value(
    const EquipmentActionSkillMergeInput& input) {
    EntityActionRecord entity{};
    entity.action_skills = {input.skill_81, input.skill_82, input.skill_83,
                            input.skill_84};
    const auto result = merge_entity_action_skill_value(
        input.current_value, entity, input.action_code);
    return {result.value, result.selected_skill};
}

namespace {

std::uint8_t bar_index(const std::uint16_t value) {
    return static_cast<std::uint8_t>(
        std::min<unsigned>(22U, value == 0U ? 0U : value - 1U));
}

std::uint8_t selected_action_code(const EquipmentCombatInput& input) {
    if (input.source_action_code.has_value()) {
        return *input.source_action_code;
    }
    if (input.selected_inventory_slot < input.inventory_item_codes.size()) {
        return input.inventory_item_codes[input.selected_inventory_slot];
    }
    return input.active_item_code;
}

std::uint16_t merge_direct_equipment_skill(const std::uint16_t current,
                                           const std::uint8_t skill) {
    const auto lower = std::min(current, static_cast<std::uint16_t>(skill));
    const auto result = static_cast<std::uint16_t>(
        lower + static_cast<std::uint16_t>(skill));
    return result == 0U ? 1U : result;
}

EntityActionRecord special_action_entity_record(
    const EquipmentCombatInput& input) {
    EntityActionRecord record{};
    record.posture = input.posture;
    record.item18_base = input.special_item18_base;
    record.attribute_7c = input.attributes[5];
    record.item_codes = input.inventory_item_codes;
    record.item_phases = input.inventory_phase_bytes;
    return record;
}

std::uint16_t apply_defense_action_modifier_impl(
    const EquipmentCombatInput& input,
    std::uint16_t value) {
    // ROM 0x55E62: the entity flag wins over the item-0x27 scan. Both are
    // word subtractions, and the source returns immediately after a match.
    if ((input.action_flags & 0x20U) != 0U) {
        return static_cast<std::uint16_t>(value - 2U);
    }
    for (std::size_t slot = 0; slot < input.inventory_item_codes.size();
         ++slot) {
        if (input.inventory_item_codes[slot] == 0x27U) {
            return static_cast<std::uint16_t>(
                value - (input.inventory_phase_bytes[slot] >> 1U));
        }
    }
    return value;
}

} // namespace

std::uint16_t apply_equipment_defense_action_modifier(
    const EquipmentCombatInput& input,
    const std::uint16_t value) {
    return apply_defense_action_modifier_impl(input, value);
}

EquipmentCombatBarResult compute_equipment_combat_bars(
    const EquipmentCombatInput& input) {
    EquipmentCombatBarResult result{};
    result.command_average = calculate_equipment_command_average(input);

    // 0x12D74 is selected only for the higher source modes. The preceding
    // 0x12FA4 call is a real action/effect service, not a bar-only shortcut.
    if (input.source_mode >= 2U && input.display_state < 0) {
        result.special_action.menu_service_called = true;
        if (input.active_item_code == 0U ||
            !input.special_magic_spell.has_value() ||
            input.special_magic_spell->spell_code !=
                static_cast<std::uint8_t>(input.active_item_code - 1U)) {
            result.special_action.magic_spell_missing = true;
        } else {
            const auto record = special_action_entity_record(input);
            const auto residual = calculate_action_residual(
                input.active_item_code, input.auxiliary_roll_bonus,
                calculate_action_effect_roll_count(record),
                *input.special_magic_spell);
            if (residual.valid) {
                result.special_action.residual_calculated = true;
                result.special_action.effect_written =
                    residual.residual_effect > 0;
                result.special_action.adjusted_roll_count =
                    residual.adjusted_roll_count;
                result.special_action.effective_difficulty =
                    residual.effective_difficulty;
                result.special_action.residual_effect =
                    residual.residual_effect;
            }
        }

        auto special_value = calculate_item18_skill_value(
            special_action_entity_record(input));
        special_value = select_posture_adjusted_value(
            special_value, input.posture, true);
        special_value = std::min<std::uint16_t>(special_value,
                                                input.roll_count_cap);
        auto special_byte = static_cast<std::uint8_t>(special_value);
        special_byte = static_cast<std::uint8_t>(
            special_byte + input.auxiliary_roll_bonus);
        const auto matching_item_code = static_cast<std::uint8_t>(
            static_cast<std::uint16_t>(input.active_item_code) - 1U +
            0x19U);
        const auto special_record = special_action_entity_record(input);
        const auto action_index = static_cast<std::uint8_t>(
            input.active_item_code - 1U);
        EntityActionPhaseModifierResult phase_modifiers{};
        const auto has_item_catalog = input.item_catalog != nullptr;
        const auto has_selector_table = input.selector_table != nullptr;
        if (has_item_catalog && has_selector_table) {
            phase_modifiers = apply_action_item_phase_modifiers(
                special_byte, special_record, matching_item_code, action_index,
                *input.item_catalog, *input.selector_table);
        } else if (has_item_catalog != has_selector_table) {
            // Do not silently fall back to the caller-owned matrix when only
            // half of the source pair is present. Preserve the phase result
            // and publish the unresolved special-item dependency.
            phase_modifiers = apply_action_item_phase_modifiers(
                special_byte, special_record, matching_item_code, action_index,
                static_cast<const EntityActionSpecialItemModifierTable*>(
                    nullptr));
            const auto slot7_item = special_record.item_codes[7];
            if (slot7_item >= 0x29U && slot7_item <= 0x2BU) {
                phase_modifiers.source_data_missing = true;
            }
        } else {
            phase_modifiers = apply_action_item_phase_modifiers(
                special_byte, special_record, matching_item_code, action_index,
                input.special_item_modifier_table.has_value()
                    ? &*input.special_item_modifier_table
                    : nullptr);
        }
        result.source_data_missing = phase_modifiers.source_data_missing;
        special_value = phase_modifiers.value;
        result.attack_value = special_value;
        result.attack_bar = bar_index(result.attack_value);
    } else {
        const auto attack_base = posture_value(result.command_average,
                                               input.posture, false);
        const auto action_code = input.source_mode >= 2U
                                     ? input.active_item_code
                                     : selected_action_code(input);
        if (input.source_mode >= 2U && input.display_state == 0x7F) {
            // 0x12DD2 reads record +0x85, which is skills[4], not the
            // neighboring +0x86 byte.
            result.attack_value = merge_direct_equipment_skill(
                attack_base, input.skills[4]);
            result.attack_bar = bar_index(result.attack_value);
        } else if (action_code < 0x0DU) {
            const auto merged = merge_equipment_action_skill_value(
                {attack_base, action_code, input.skills[0], input.skills[1],
                 input.skills[2], input.skills[3]});
            result.attack_value = merged.value;
            result.attack_bar = bar_index(result.attack_value);
        } else if (action_code < 0x10U) {
            // 0x12D46 selects record +0x87 directly.
            result.attack_value = merge_direct_equipment_skill(
                attack_base, input.skills[6]);
            result.attack_bar = bar_index(result.attack_value);
        } else if (input.source_mode < 2U && action_code == 0x17U) {
            // 0x12DD2 selects record +0x85 for the special action row.
            result.attack_value = merge_direct_equipment_skill(
                attack_base, input.skills[4]);
            result.attack_bar = bar_index(result.attack_value);
        } else if (input.source_mode >= 2U) {
            // 0x12D46 is also the higher-mode fallback for an equipped
            // action code at or above thirteen.
            result.attack_value = merge_direct_equipment_skill(
                attack_base, input.skills[6]);
            result.attack_bar = bar_index(result.attack_value);
        } else {
            // The ROM exits the lower-mode attack path for 0x10..0x16 and
            // does not publish an attack strip for those action codes.
            result.attack_written = false;
        }
    }

    // 0x12E12 applies this rejection only in the lower source modes. In the
    // higher mode, the special action path proceeds directly to defense.
    const auto action_code = selected_action_code(input);
    if (input.source_mode < 2U && action_code != 0x17U &&
        action_code >= 0x10U) {
        result.defense_written = false;
        return result;
    }

    auto defense_value = posture_value(result.command_average, input.posture,
                                       true);
    defense_value = static_cast<std::uint16_t>(
        defense_value + input.attributes[0]);
    defense_value = apply_equipment_defense_action_modifier(
        input, defense_value);
    unsigned cyberware_adjustment = 9U;
    for (unsigned bit = 9U; bit <= 11U; ++bit) {
        if ((input.cyberware_flags & (std::uint16_t{1U} << bit)) != 0U) {
            cyberware_adjustment -= 8U;
        } else {
            ++cyberware_adjustment;
        }
    }
    result.defense_value = static_cast<std::uint16_t>(
        defense_value + cyberware_adjustment);
    result.defense_bar = bar_index(result.defense_value);
    return result;
}

EquipmentHighlightResult render_equipment_highlights(
    const EquipmentHighlightInput& input,
    const EquipmentHighlightCallbacks& callbacks) {
    EquipmentHighlightResult result{};
    // ROM 0x11624 begins with 0x116C4's four-grid/metadata batch.
    call_once(callbacks.write_menu_grid_quad);
    if (input.equipped_display_state >= 0 &&
        input.equipped_display_state != 0x7F) {
        call_once(callbacks.write_menu_grid);
        result.equipped_grid_written = true;
    }

    if (input.highlight_item != 0U) {
        const auto target = static_cast<std::uint8_t>(
            static_cast<std::uint16_t>(input.highlight_item) + 0x2BU);
        for (std::size_t slot = input.inventory_item_codes.size(); slot-- > 0;) {
            if (input.inventory_item_codes[slot] != target) {
                continue;
            }
            call_once(callbacks.write_menu_grid);
            result.inventory_grid_written = true;
            result.matched_inventory_slot = static_cast<std::int8_t>(slot);
            break;
        }
    }
    call_once(callbacks.render_menu_tile_rows);
    return result;
}

namespace {

void publish_item_detail_grid(
    ItemActionDetailResult& result,
    const ItemActionDetailGridPlan& plan,
    const std::function<void(const ItemActionDetailGridPlan&)>& callback) {
    result.grid_requests.push_back(plan);
    if (callback) {
        callback(plan);
    }
}

void publish_item_detail_text(
    ItemActionDetailResult& result,
    const ItemActionDetailTextPlan& plan,
    const std::function<void(const ItemActionDetailTextPlan&)>& callback) {
    result.text_requests.push_back(plan);
    if (callback) {
        callback(plan);
    }
}

ActionDetailLabelVariant item_action_label_for_code(
    const std::uint8_t action_code) {
    if (action_code < 0x0DU) {
        return ActionDetailLabelVariant::ordinary;
    }
    if (action_code < 0x14U) {
        return ActionDetailLabelVariant::extended;
    }
    if (action_code == 0x14U ||
        (action_code >= 0x18U && action_code <= 0x28U)) {
        return ActionDetailLabelVariant::action_dependent;
    }
    return ActionDetailLabelVariant::none;
}

bool item_action_has_no_label(const std::uint8_t action_code) {
    return (action_code >= 0x15U && action_code <= 0x17U) ||
           (action_code >= 0x29U && action_code <= 0x2BU);
}

} // namespace

ItemActionDetailResult render_item_action_details(
    const ItemActionDetailInput& input,
    ResourceVariantState& resource_variant,
    const ItemActionDetailCallbacks& callbacks) {
    ItemActionDetailResult result{};
    result.returned_item_code = input.item_code;

    // 0x12A54 runs before the item row is used. A zero item code is not a
    // valid one-based item row, so retain the source D0 result but do not
    // manufacture a wrapped host index.
    const auto previous_variant = resource_variant.alternate;
    update_resource_variant(resource_variant,
                            static_cast<std::int16_t>(input.item_code),
                            callbacks.upload_resource_stream);
    result.resource_variant_changed =
        previous_variant != resource_variant.alternate;
    if (input.item_code == 0U) {
        result.early_return = true;
        return result;
    }

    const auto item_record_index =
        static_cast<std::size_t>(input.item_code - 1U);
    if (input.item_art_available) {
        // 0x12AA8: item art at row +0x1E, D4=-0x3E5C (0xC1A4),
        // D1=4, D7=8, palette flags 0x2000.
        publish_item_detail_grid(
            result,
            ItemActionDetailGridPlan{ItemActionDetailGridKind::item_art,
                                     0U,
                                     0xC1A4U,
                                     4U,
                                     8U,
                                     0x2000U,
                                     0U},
            callbacks.write_grid);
        result.item_art_written = true;
    }

    // 0x12AC8 always renders the selected item's name from its 0x30-byte
    // record, independent of whether an art pointer was present.
    publish_item_detail_text(
        result,
        ItemActionDetailTextPlan{ItemActionDetailTextKind::item_name,
                                 item_record_index,
                                 ActionDetailLabelVariant::none,
                                 std::nullopt,
                                 0xC420U,
                                 0x6000U},
        callbacks.render_text);
    result.item_name_rendered = true;

    if (!callbacks.read_entity_action_code) {
        // 0x12148 is a real record lookup, not a safe default of zero. Keep
        // this incomplete boundary visible to the caller.
        result.action_lookup_missing = true;
        return result;
    }
    result.action_code = callbacks.read_entity_action_code();

    const auto phase_slot = input.item_art_available ? 8U : input.phase_slot;
    if (phase_slot >= input.phase_bytes.size()) {
        result.action_lookup_missing = true;
        return result;
    }
    result.phase_slot_used = static_cast<std::uint8_t>(phase_slot);

    const auto action_code = *result.action_code;
    if (action_code < 0x0DU) {
        // 0x12AE4-0x12B86: phase overlays are checked in the source order
        // 1, 2, 0, 3, 4, 5. Each pair shares a source destination.
        constexpr std::array<std::uint8_t, 6U> kPhaseBitOrder{
            1U, 2U, 0U, 3U, 4U, 5U};
        constexpr std::array<std::uint16_t, 6U> kPhaseDestinations{
            0xC19CU, 0xC19CU, 0xC1B6U, 0xC1B6U, 0xC336U, 0xC336U};
        for (std::size_t index = 0; index < kPhaseBitOrder.size();
             ++index) {
            const auto phase_bit = kPhaseBitOrder[index];
            if ((input.phase_bytes[phase_slot] &
                 (std::uint8_t{1U} << phase_bit)) == 0U) {
                continue;
            }
            publish_item_detail_grid(
                result,
                ItemActionDetailGridPlan{
                    ItemActionDetailGridKind::phase_overlay,
                    phase_bit,
                    kPhaseDestinations[index],
                    1U,
                    3U,
                    0x2000U,
                    0U},
                callbacks.write_grid);
        }
        result.label = ActionDetailLabelVariant::ordinary;
    } else {
        if (item_action_has_no_label(action_code)) {
            // 0x12B8E's 0x15..0x17 and 0x29..0x2B bands return before the
            // label/value grids are emitted.
            result.early_return = true;
            return result;
        }
        if (action_code >= 0x2CU) {
            // The source hands the selection to 0x11352 and returns without
            // rendering a label/value pair.
            if (callbacks.apply_armor_selection) {
                callbacks.apply_armor_selection(true);
            }
            result.armor_selection_requested = true;
            return result;
        }

        result.label = item_action_label_for_code(action_code);
        if (!callbacks.read_entity_action_code) {
            result.action_lookup_missing = true;
            return result;
        }
        // 0x12BE4 repeats 0x12148 for the extended and action-dependent
        // label tables before selecting active-record +0x6E + D7.
        result.label_action_code = callbacks.read_entity_action_code();
    }

    const auto detail_value = result.label == ActionDetailLabelVariant::ordinary
                                  ? input.item_detail_value
                                  : input.phase_bytes[phase_slot];

    publish_item_detail_text(
        result,
        ItemActionDetailTextPlan{ItemActionDetailTextKind::action_label,
                                 item_record_index,
                                 result.label,
                                 result.label == ActionDetailLabelVariant::ordinary
                                     ? result.action_code
                                     : result.label_action_code,
                                 0xC31CU,
                                 0x4000U},
        callbacks.render_text);
    result.action_label_rendered = true;

    // 0x12BF0 selects the detail-value asset through the source's decoded
    // table. The host plan carries the value, not the original 0x6AC46
    // pointer arithmetic.
    publish_item_detail_grid(
        result,
        ItemActionDetailGridPlan{ItemActionDetailGridKind::detail_value,
                                 0U,
                                 0xC39EU,
                                 0U,
                                 1U,
                                 0x2000U,
                                 detail_value},
        callbacks.write_grid);
    result.detail_value_written = true;
    return result;
}

ItemActionDetailResult render_item_action_details(
    const ItemActionDetailInput& input,
    const UsItemCatalog& item_catalog,
    ResourceVariantState& resource_variant,
    const ItemActionDetailCallbacks& callbacks) {
    ItemActionDetailResult missing{};
    missing.returned_item_code = input.item_code;

    // 0x12A8C is reached with an ordinary one-based inventory/item code. A
    // missing row is a source-data problem, not permission to wrap a host
    // index or reuse the caller's guessed art/value fields.
    if (input.item_code == 0U ||
        static_cast<std::size_t>(input.item_code) > item_catalog.size()) {
        missing.item_record_missing = true;
        return missing;
    }

    const auto& item = item_catalog[static_cast<std::size_t>(
        input.item_code - 1U)];
    if (item.item_code != input.item_code) {
        missing.item_record_missing = true;
        return missing;
    }

    auto source_bound_input = input;
    source_bound_input.item_art_available = item.has_detail_art;
    source_bound_input.item_detail_value = item.source_combat_factor;
    return render_item_action_details(source_bound_input, resource_variant,
                                      callbacks);
}

EquipmentActionDetailRowResult render_equipment_action_detail_row(
    const EquipmentActionDetailRowInput& input,
    const EquipmentActionDetailRowCallbacks& callbacks) {
    EquipmentActionDetailRowResult result{};
    std::uint8_t detail_asset_index = 0U;
    auto asset_table = EquipmentActionDetailAssetTable::normal_item_records;
    std::optional<std::int8_t> source_detail_asset_index =
        input.detail_asset_index;

    const auto no_active_action = input.active_action_code == 0U;
    if (no_active_action) {
        // 0x12EBE-0x12EDE chooses a small detail asset from cyberware flags
        // before entering 0x13000. Bit 3 wins over bit 2; with neither bit,
        // the source renders the header and uses asset index one.
        if ((input.cyberware_flags & (std::uint16_t{1U} << 3U)) != 0U) {
            detail_asset_index = 1U;
        } else if ((input.cyberware_flags & (std::uint16_t{1U} << 2U)) !=
                   0U) {
            detail_asset_index = 0U;
        } else {
            const EquipmentActionDetailHeaderPlan header{};
            if (callbacks.render_action_header) {
                callbacks.render_action_header(header);
            }
            result.action_header_rendered = true;
            detail_asset_index = 1U;
        }
        asset_table = EquipmentActionDetailAssetTable::no_active_action;
    } else if (input.display_state < 0 && input.source_mode >= 2U) {
        // 0x12F00: the higher-mode negative-display path shows the same
        // header for action index four (active code five), advances the
        // action menu, then reads the alternate D4EA0 row.
        if (input.active_action_code == 5U) {
            const EquipmentActionDetailHeaderPlan header{};
            if (callbacks.render_action_header) {
                callbacks.render_action_header(header);
            }
            result.action_header_rendered = true;
        }
        if (callbacks.advance_entity_action_menu) {
            callbacks.advance_entity_action_menu();
        }
        result.action_menu_advanced = true;
        asset_table = EquipmentActionDetailAssetTable::alternate_action_records;

        const auto expected_spell_code = static_cast<std::uint8_t>(
            input.active_action_code - 1U);
        if (!input.magic_spell.has_value() ||
            input.magic_spell->spell_code != expected_spell_code) {
            // ROM 0x12F36 indexes D4EA0 by the active action code. A missing
            // or mismatched typed row is an unresolved source-data boundary;
            // do not fall back to a caller-provided byte from another table.
            result.detail_asset_missing = true;
            return result;
        }
        source_detail_asset_index = input.magic_spell->availability_byte;
    }

    if (!no_active_action) {
        if (!source_detail_asset_index.has_value()) {
            // The row +0x25 read is source data, not a safe default.
            result.detail_asset_missing = true;
            return result;
        }
        if (*source_detail_asset_index < 0) {
            // 0x12F46/0x12F4A return before the 0x13000 selection and final
            // detail row when the signed source byte is negative.
            result.early_return = true;
            return result;
        }
        detail_asset_index = static_cast<std::uint8_t>(
            *source_detail_asset_index);
    }

    EntityActionDetailInput selection_input{};
    selection_input.source_mode = input.source_mode;
    selection_input.selected_action_code = input.selected_action_code;
    selection_input.action_detail_code = input.action_detail_code;
    selection_input.display_state = input.display_state;
    selection_input.active_action_code = input.active_action_code;
    selection_input.auxiliary_value = input.auxiliary_value;
    selection_input.cyberware_base = input.cyberware_base;
    selection_input.cyberware_flags = input.cyberware_flags;
    const auto selection = select_entity_action_detail(selection_input);
    if (!selection.has_value()) {
        // 0x13000's record/table lookup is an explicit conversion boundary.
        result.action_selection_missing = true;
        return result;
    }
    result.action_selection = selection;
    if (callbacks.publish_action_selection) {
        callbacks.publish_action_selection(*selection);
    }

    const EquipmentActionDetailTextPlan text_plan{};
    if (callbacks.render_detail_text) {
        callbacks.render_detail_text(text_plan);
    }
    result.detail_text_rendered = true;

    const EquipmentActionDetailGridPlan grid_plan{
        asset_table,
        detail_asset_index,
        0xC52AU,
        0U,
        6U,
        0x2000U};
    result.detail_grid = grid_plan;
    if (callbacks.write_grid) {
        callbacks.write_grid(grid_plan);
    }
    result.detail_grid_written = true;
    return result;
}

EquipmentActionDetailRowResult render_equipment_action_detail_row(
    const EquipmentActionDetailRowInput& input,
    const UsItemCatalog& item_catalog,
    const EquipmentActionDetailRowCallbacks& callbacks) {
    auto source_bound_input = input;
    const auto ordinary_active_action =
        input.active_action_code != 0U &&
        !(input.display_state < 0 && input.source_mode >= 2U);
    if (ordinary_active_action) {
        if (input.active_action_code > item_catalog.size()) {
            EquipmentActionDetailRowResult missing{};
            missing.detail_asset_missing = true;
            return missing;
        }
        const auto& item = item_catalog[static_cast<std::size_t>(
            input.active_action_code - 1U)];
        if (item.item_code != input.active_action_code) {
            EquipmentActionDetailRowResult missing{};
            missing.detail_asset_missing = true;
            return missing;
        }
        source_bound_input.detail_asset_index = item.signed_detail_asset_index;
    }
    return render_equipment_action_detail_row(source_bound_input, callbacks);
}

namespace {

void publish_equipment_details_text(
    EquipmentDetailsResult& result,
    const EquipmentDetailsTextPlan& plan,
    const std::function<void(const EquipmentDetailsTextPlan&)>& callback) {
    result.text_requests.push_back(plan);
    if (callback) {
        callback(plan);
    }
}

void publish_equipment_meter(
    EquipmentDetailsResult& result,
    const EquipmentMeterPlan& plan,
    const std::function<void(const EquipmentMeterPlan&)>& callback) {
    result.meter_requests.push_back(plan);
    if (callback) {
        callback(plan);
    }
}

} // namespace

namespace {

EquipmentDetailsResult render_equipment_details_impl(
    const EquipmentDetailsInput& input,
    const EquipmentDetailsCallbacks& callbacks,
    const UsItemCatalog* item_catalog) {
    EquipmentDetailsResult result{};
    if (!input.refresh_requested) {
        if (callbacks.render_existing_menu_row) {
            callbacks.render_existing_menu_row();
        }
        result.existing_menu_row_rendered = true;
        return result;
    }

    result.refresh_flag_cleared = true;
    if (callbacks.prepare_menu_grid_variant_b) {
        callbacks.prepare_menu_grid_variant_b();
    }
    result.menu_grid_prepared = true;

    publish_equipment_details_text(
        result,
        EquipmentDetailsTextPlan{EquipmentDetailsTextKind::heading,
                                 0U,
                                 0xC2A0U,
                                 0x6000U},
        callbacks.render_text);
    publish_equipment_details_text(
        result,
        EquipmentDetailsTextPlan{EquipmentDetailsTextKind::runner_race,
                                 input.race_or_state,
                                 0xC1A0U,
                                 0x6000U},
        callbacks.render_text);
    publish_equipment_details_text(
        result,
        EquipmentDetailsTextPlan{EquipmentDetailsTextKind::runner_class,
                                 input.class_code,
                                 0xC1ACU,
                                 0x6000U},
        callbacks.render_text);

    const auto active_action_code = input.action_row.active_action_code;
    if (active_action_code == 0U) {
        const auto cyberware_flags = input.action_row.cyberware_flags;
        if ((cyberware_flags & (std::uint16_t{1U} << 3U)) != 0U) {
            publish_equipment_details_text(
                result,
                EquipmentDetailsTextPlan{
                    EquipmentDetailsTextKind::cyberware_action_bit3,
                    0U,
                    0xC422U,
                    0x6000U},
                callbacks.render_text);
        } else if ((cyberware_flags & (std::uint16_t{1U} << 2U)) != 0U) {
            publish_equipment_details_text(
                result,
                EquipmentDetailsTextPlan{
                    EquipmentDetailsTextKind::cyberware_action_bit2,
                    0U,
                    0xC422U,
                    0x6000U},
                callbacks.render_text);
        } else {
            publish_equipment_details_text(
                result,
                EquipmentDetailsTextPlan{EquipmentDetailsTextKind::no_action,
                                         0U,
                                         0xC420U,
                                         0x6000U},
                callbacks.render_text);
        }
    } else {
        const auto item_row = static_cast<std::size_t>(active_action_code -
                                                       1U);
        const auto text_kind = input.action_row.display_state < 0
                                   ? EquipmentDetailsTextKind::
                                         active_action_alternate
                                   : EquipmentDetailsTextKind::
                                         active_action_normal;
        publish_equipment_details_text(
            result,
            EquipmentDetailsTextPlan{text_kind, item_row, 0xC420U, 0x6000U},
            callbacks.render_text);
    }

    // 0x11304 uses CMP.B #0x10 followed by BPL. Preserve its wrapped byte
    // sign test instead of treating the action byte as a host signed int.
    const auto detail_gate = static_cast<std::uint8_t>(
        active_action_code - 0x10U);
    if ((detail_gate & 0x80U) != 0U) {
        auto row_input = input.action_row;
        row_input.source_mode = input.action_row.source_mode;
        row_input.display_state = input.action_row.display_state;
        row_input.active_action_code = active_action_code;
        if (item_catalog != nullptr) {
            result.action_detail_row = render_equipment_action_detail_row(
                row_input, *item_catalog, callbacks.action_row_callbacks);
        } else {
            result.action_detail_row = render_equipment_action_detail_row(
                row_input, callbacks.action_row_callbacks);
        }
        result.action_detail_row_invoked = true;
    }

    publish_equipment_meter(
        result,
        EquipmentMeterPlan{EquipmentMeterKind::physical,
                            input.physical_meter,
                            0xC2B4U,
                            0x6000U},
        callbacks.build_meter_glyphs);
    result.physical_meter_rendered = true;
    publish_equipment_meter(
        result,
        EquipmentMeterPlan{EquipmentMeterKind::mental,
                            input.mental_meter,
                            0xC334U,
                            0x6000U},
        callbacks.build_meter_glyphs);
    result.mental_meter_rendered = true;

    if (callbacks.apply_armor_selection) {
        callbacks.apply_armor_selection();
    }
    result.armor_selection_applied = true;
    if (callbacks.prepare_menu_dialog) {
        callbacks.prepare_menu_dialog();
    }
    result.dialog_prepared = true;
    return result;
}

} // namespace

EquipmentDetailsResult render_equipment_details(
    const EquipmentDetailsInput& input,
    const EquipmentDetailsCallbacks& callbacks) {
    return render_equipment_details_impl(input, callbacks, nullptr);
}

EquipmentDetailsResult render_equipment_details(
    const EquipmentDetailsInput& input,
    const UsItemCatalog& item_catalog,
    const EquipmentDetailsCallbacks& callbacks) {
    return render_equipment_details_impl(input, callbacks, &item_catalog);
}

} // namespace shadowrun::ghidra_conversion
