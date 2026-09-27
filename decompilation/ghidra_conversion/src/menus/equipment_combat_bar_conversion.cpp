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

std::uint8_t inventory_phase_for_code(
    const EquipmentCombatInput& input,
    const std::uint8_t item_code) {
    for (std::size_t slot = 0; slot < input.inventory_item_codes.size();
         ++slot) {
        if (input.inventory_item_codes[slot] == item_code) {
            return input.inventory_phase_bytes[slot];
        }
    }
    return 0U;
}

bool source_byte_subtraction_is_nonnegative(const std::uint8_t minuend,
                                            const std::uint8_t subtrahend) {
    const auto result = static_cast<std::uint8_t>(minuend - subtrahend);
    return (result & 0x80U) == 0U;
}

} // namespace

std::uint16_t calculate_equipment_command_average(
    const EquipmentCombatInput& input) {
    // 0x55ED6 uses ADD.B for all three attributes and for the item phase.
    // Keep the low-byte wrap instead of doing a wider host sum.
    auto attribute_sum = static_cast<std::uint8_t>(0U);
    attribute_sum = static_cast<std::uint8_t>(
        attribute_sum + input.attributes[1]);
    attribute_sum = static_cast<std::uint8_t>(
        attribute_sum + input.attributes[4]);
    attribute_sum = static_cast<std::uint8_t>(
        attribute_sum + input.attributes[5]);
    attribute_sum = static_cast<std::uint8_t>(attribute_sum >> 1U);
    attribute_sum = static_cast<std::uint8_t>(
        attribute_sum + inventory_phase_for_code(input, 0x28U));
    return attribute_sum;
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
    // 0x12C74 branches on the sign bit produced by CMP.B, not on a host
    // signed/unsigned relational comparison. This matters for wrapped byte
    // differences such as 0x01 - 0x80 = 0x81.
    auto selected_skill = input.skill_81;
    const auto first_band = !source_byte_subtraction_is_nonnegative(
        input.action_code, 0x08U);
    const auto second_band = !source_byte_subtraction_is_nonnegative(
        input.action_code, 0x0BU);
    const auto candidate = first_band
                               ? input.skill_82
                               : second_band ? input.skill_83 : input.skill_84;
    if (!source_byte_subtraction_is_nonnegative(selected_skill, candidate)) {
        selected_skill = candidate;
    }

    // The final CMP.W is followed by BPL. Callers clear D7.W before entering
    // this leaf, so the selected byte is a zero-extended word. Preserve the
    // source's 16-bit subtraction sign test and addition wrap instead of
    // replacing it with std::min plus an unbounded sum.
    const auto selected_word = static_cast<std::uint16_t>(selected_skill);
    const auto comparison = static_cast<std::uint16_t>(
        selected_word - input.current_value);
    auto lower = input.current_value;
    if ((comparison & 0x8000U) != 0U) {
        lower = selected_word;
    }
    const auto result = static_cast<std::uint16_t>(lower + selected_word);
    return {static_cast<std::uint16_t>(result == 0U ? 1U : result),
            selected_skill};
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

EntityActionRecord special_action_record(const EquipmentCombatInput& input) {
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
        if (input.special_action_record.has_value() &&
            input.active_item_code != 0U) {
            const auto record = special_action_record(input);
            const auto residual = calculate_action_residual(
                input.active_item_code, input.auxiliary_roll_bonus,
                calculate_action_effect_roll_count(record),
                *input.special_action_record);
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
            special_action_record(input));
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
        const auto phase_modifiers = apply_action_item_phase_modifiers(
            special_byte, special_action_record(input), matching_item_code,
            static_cast<std::uint8_t>(input.active_item_code - 1U),
            input.special_item_modifier_table.has_value()
                ? &*input.special_item_modifier_table
                : nullptr);
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

EquipmentActionMetricResult refresh_equipment_action_metric(
    const EquipmentActionMetricInput& input,
    const EquipmentActionMetricCallbacks& callbacks) {
    EquipmentActionMetricResult result{};
    // ROM 0x105E6 enters 0x10660 with D4=current force and D7=action code.
    const auto residual = calculate_action_residual(
        input.action_code, input.current_force,
        calculate_action_effect_roll_count(input.entity), input.action);
    result.residual_valid = residual.valid;
    result.residual_effect = residual.residual_effect;
    if (!residual.valid || residual.residual_effect <= 0) {
        if (callbacks.render_no_effect_text) {
            callbacks.render_no_effect_text();
            result.no_effect_text_rendered = true;
        }
        return result;
    }

    result.effect_grid_row = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(residual.residual_effect) - 1U);
    if (callbacks.write_effect_grid) {
        callbacks.write_effect_grid(result.effect_grid_row);
        result.effect_grid_written = true;
    }
    return result;
}

EquipmentCombatValuesResult refresh_equipment_combat_values(
    const EquipmentCombatValuesInput& input,
    const EquipmentCombatValuesCallbacks& callbacks) {
    EquipmentCombatValuesResult result{};
    if (!input.alternate_path) {
        // ROM 0x1055E/D0=0: D7=0 selects the posture-reduction component.
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
        result.primary_value = value;
    } else {
        // ROM 0x1055E/D0!=0 enters 0x1047E before adding +0x77/+0x7C.
        call_once(callbacks.refresh_ready_entity_animations);
        const auto ready_bonus = calculate_ready_entity_action_bonus(
            input.ready_entities);
        result.primary_value = static_cast<std::uint8_t>(
            ready_bonus + input.attribute_77);
        result.secondary_value = static_cast<std::uint8_t>(
            ready_bonus + input.attribute_7c);
    }

    const auto primary_row = result.primary_value == 0U
                                 ? 0U
                                 : static_cast<std::uint16_t>(
                                       result.primary_value - 1U);
    result.grid_requests.emplace_back(EquipmentCombatGrid::primary,
                                      primary_row);
    if (callbacks.write_grid) {
        callbacks.write_grid(EquipmentCombatGrid::primary, primary_row);
    }

    if (result.secondary_value != 0U) {
        const auto secondary_row = static_cast<std::uint16_t>(
            result.secondary_value - 1U);
        result.grid_requests.emplace_back(EquipmentCombatGrid::secondary,
                                          secondary_row);
        if (callbacks.write_grid) {
            callbacks.write_grid(EquipmentCombatGrid::secondary,
                                 secondary_row);
        }
    }
    return result;
}

} // namespace shadowrun::ghidra_conversion
