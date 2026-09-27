#include "shadowrun_ghidra/menus/equipment_combat_bar_conversion.hpp"

#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    const auto action_skill = merge_equipment_action_skill_value(
        EquipmentActionSkillMergeInput{4U, 7U, 1U, 3U, 8U, 2U});
    const auto action_skill_next_band = merge_equipment_action_skill_value(
        EquipmentActionSkillMergeInput{4U, 8U, 1U, 3U, 8U, 2U});
    const auto action_skill_final_band = merge_equipment_action_skill_value(
        EquipmentActionSkillMergeInput{4U, 11U, 1U, 3U, 8U, 2U});
    const auto action_skill_zero = merge_equipment_action_skill_value(
        EquipmentActionSkillMergeInput{0U, 0U, 0U, 0U, 0U, 0U});
    if (action_skill.selected_skill != 3U || action_skill.value != 6U ||
        action_skill_next_band.selected_skill != 8U ||
        action_skill_next_band.value != 12U ||
        action_skill_final_band.selected_skill != 2U ||
        action_skill_final_band.value != 4U || action_skill_zero.value != 1U) {
        return 100;
    }

    // 0x12C74 uses wrapped CMP.B predicates. 0xFF falls through the first
    // action band, and 0x01 - 0x80 has its byte sign bit set, selecting 0x80.
    const auto wrapped_action = merge_equipment_action_skill_value(
        EquipmentActionSkillMergeInput{4U, 0xFFU, 1U, 2U, 3U, 4U});
    const auto wrapped_skill = merge_equipment_action_skill_value(
        EquipmentActionSkillMergeInput{1U, 0U, 1U, 0x80U, 3U, 4U});
    const auto wrapped_word = merge_equipment_action_skill_value(
        EquipmentActionSkillMergeInput{0xFFFFU, 0x0BU, 0xFFU, 0U, 0U, 0xFFU});
    if (wrapped_action.selected_skill != 2U || wrapped_action.value != 4U ||
        wrapped_skill.selected_skill != 0x80U || wrapped_skill.value != 0x81U ||
        wrapped_word.value != 0x00FEU) {
        return 101;
    }

    EquipmentCombatInput input{};
    input.attributes = {2U, 4U, 0U, 0U, 4U, 4U, 0U, 0U};
    input.skills[1] = 2U;
    input.skills[2] = 3U;
    input.inventory_item_codes[0] = 0x28U;
    input.inventory_phase_bytes[0] = 2U;
    input.inventory_item_codes[6] = 2U;
    input.display_state = 0;
    input.active_item_code = 2U;
    input.posture = 2U;
    input.selected_inventory_slot = 6U;

    const auto result = compute_equipment_combat_bars(input);
    if (result.command_average != 8U || result.attack_value != 4U ||
        result.attack_bar != 3U || result.defense_value != 18U ||
        result.defense_bar != 17U || !result.attack_written ||
        !result.defense_written) {
        return 1;
    }

    input.source_mode = 2U;
    input.display_state = 0x7F;
    input.skills[4] = 4U;
    const auto unarmed = compute_equipment_combat_bars(input);
    if (unarmed.attack_value != 8U || unarmed.attack_bar != 7U) {
        return 2;
    }

    input.inventory_item_codes[6] = 0x10U;
    const auto utility_selected = compute_equipment_combat_bars(input);
    if (!utility_selected.defense_written ||
        utility_selected.attack_bar != unarmed.attack_bar) {
        return 3;
    }

    input.source_mode = 0U;
    input.source_action_code = 2U;
    input.display_state = -1;
    const auto special = compute_equipment_combat_bars(input);
    if (!special.attack_written || !special.defense_written ||
        special.special_action.menu_service_called) {
        return 4;
    }

    input.source_mode = 2U;
    input.roll_count_cap = 12U;
    input.auxiliary_roll_bonus = 3U;
    input.special_item18_base = 20U;
    input.inventory_item_codes[0] = 0x18U;
    input.inventory_phase_bytes[0] = 2U;
    input.inventory_item_codes[1] = 0x1EU;
    input.inventory_phase_bytes[1] = 4U;
    input.inventory_item_codes[6] = 0x10U;
    input.active_item_code = 6U;
    input.special_action_record = EntityActionCatalogRecord{1U, 30};
    EntityActionSpecialItemModifierTable special_item_modifiers{};
    special_item_modifiers.action_offsets[5] = 1;
    special_item_modifiers.slot7_item_modifiers[0][1] = -2;
    input.inventory_item_codes[7] = 0x29U;
    input.special_item_modifier_table = special_item_modifiers;
    const auto special_mode = compute_equipment_combat_bars(input);
    if (!special_mode.attack_written || !special_mode.defense_written ||
        special_mode.attack_value != 16U || special_mode.attack_bar != 15U ||
        !special_mode.special_action.menu_service_called ||
        !special_mode.special_action.residual_calculated ||
        !special_mode.special_action.effect_written ||
        special_mode.special_action.adjusted_roll_count != 6U ||
        special_mode.special_action.residual_effect != 24) {
        return 5;
    }

    input.display_state = 0;
    input.active_item_code = 0x10U;
    const auto unsupported_action = compute_equipment_combat_bars(input);
    if (!unsupported_action.attack_written ||
        unsupported_action.attack_value != 1U ||
        unsupported_action.attack_bar != 0U ||
        !unsupported_action.defense_written) {
        return 6;
    }

    input.source_mode = 0U;
    input.action_flags = 0x20U;
    input.inventory_item_codes[0] = 0U;
    input.inventory_phase_bytes[0] = 0U;
    input.selected_inventory_slot = 2U;
    input.active_item_code = 2U;
    const auto flagged_defense = compute_equipment_combat_bars(input);
    if (flagged_defense.defense_value != 15U) {
        return 7;
    }

    EquipmentActionMetricInput metric_input{};
    metric_input.action_code = 1U;
    metric_input.current_force = 2U;
    metric_input.entity.item18_base = 10U;
    metric_input.action.base_effect = 20;
    std::vector<int> metric_events;
    const auto metric = refresh_equipment_action_metric(
        metric_input,
        {[&] { metric_events.push_back(1); },
         [&](const std::uint16_t row) {
             metric_events.push_back(static_cast<int>(row));
         }});
    if (!metric.residual_valid || metric.residual_effect != 15 ||
        !metric.effect_grid_written || metric.no_effect_text_rendered ||
        metric.effect_grid_row != 14U ||
        metric_events != std::vector<int>{14}) {
        return 8;
    }

    metric_input.action.base_effect = 1;
    metric_events.clear();
    const auto no_effect = refresh_equipment_action_metric(
        metric_input,
        {[&] { metric_events.push_back(1); },
         [&](const std::uint16_t row) {
             metric_events.push_back(static_cast<int>(row));
         }});
    if (!no_effect.residual_valid || no_effect.residual_effect != -4 ||
        !no_effect.no_effect_text_rendered || no_effect.effect_grid_written ||
        metric_events != std::vector<int>{1}) {
        return 9;
    }

    EquipmentCombatValuesInput combat_values{};
    combat_values.current_force = 2U;
    combat_values.action_cap = 5U;
    combat_values.active_entity.item18_base = 12U;
    combat_values.active_entity.posture = 2U;
    std::vector<int> combat_value_events;
    EquipmentCombatValuesCallbacks combat_value_callbacks{};
    combat_value_callbacks.resolve_menu_metric =
        [&](const std::uint16_t value) {
            combat_value_events.push_back(1);
            return std::optional<std::uint16_t>{
                static_cast<std::uint16_t>(value + 3U)};
        };
    combat_value_callbacks.write_grid =
        [&](const EquipmentCombatGrid grid, const std::uint16_t row) {
            combat_value_events.push_back(
                grid == EquipmentCombatGrid::primary
                    ? static_cast<int>(row + 10U)
                    : static_cast<int>(row + 20U));
        };
    const auto combat_values = refresh_equipment_combat_values(
        combat_values, combat_value_callbacks);
    if (!expect(combat_values.menu_metric_available &&
                combat_values.primary_value == 10U &&
                combat_values.secondary_value == 0U &&
                combat_values.grid_requests.size() == 1U &&
                combat_values.grid_requests[0].second == 9U &&
                combat_value_events == std::vector<int>{1, 19})) {
        return 10;
    }

    combat_values = {};
    combat_values.alternate_path = true;
    combat_values.attribute_77 = 4U;
    combat_values.attribute_7c = 2U;
    combat_values.ready_entities[0].flags = 1U;
    combat_values.ready_entities[0].class_code = 0U;
    combat_values.ready_entities[0].item18_base = 8U;
    combat_values.ready_entities[1].flags = 1U;
    combat_values.ready_entities[1].class_code = 1U;
    combat_values.ready_entities[1].item18_base = 5U;
    combat_value_events.clear();
    int ready_refreshes = 0;
    combat_value_callbacks.resolve_menu_metric = {};
    combat_value_callbacks.refresh_ready_entity_animations =
        [&] { ++ready_refreshes; };
    const auto alternate_values = refresh_equipment_combat_values(
        combat_values, combat_value_callbacks);
    if (!expect(ready_refreshes == 1 &&
                alternate_values.primary_value == 7U &&
                alternate_values.secondary_value == 5U &&
                alternate_values.grid_requests.size() == 2U &&
                alternate_values.grid_requests[0].second == 6U &&
                alternate_values.grid_requests[1].second == 4U &&
                combat_value_events == std::vector<int>{16, 24})) {
        return 11;
    }

    return 0;
}
