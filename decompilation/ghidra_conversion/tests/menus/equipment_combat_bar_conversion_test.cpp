#include "shadowrun_ghidra/menus/equipment_combat_bar_conversion.hpp"

#include <cstdint>

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

    return 0;
}
