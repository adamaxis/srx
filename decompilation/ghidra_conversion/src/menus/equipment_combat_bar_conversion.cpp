#include "shadowrun_ghidra/menus/equipment_combat_bar_conversion.hpp"

#include <algorithm>
#include <cstddef>

namespace shadowrun::ghidra_conversion {
namespace {

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

std::uint16_t merge_equipment_action_skill_value(
    const std::uint16_t current,
    const std::uint16_t skill) {
    const auto lower = std::min(current, skill);
    const auto result = static_cast<std::uint16_t>(lower + skill);
    return result == 0U ? 1U : result;
}

namespace {

std::uint8_t bar_index(const std::uint16_t value) {
    return static_cast<std::uint8_t>(
        std::min<unsigned>(22U, value == 0U ? 0U : value - 1U));
}

std::optional<std::uint16_t> selected_skill_for_action(
    const EquipmentCombatInput& input,
    const std::uint8_t action_code) {
    if (action_code < 0x0DU) {
        auto skill = static_cast<std::uint16_t>(input.skills[1]);
        if (action_code < 0x08U) {
            skill = std::max(skill, static_cast<std::uint16_t>(input.skills[2]));
        } else if (action_code < 0x0BU) {
            skill = std::max(skill, static_cast<std::uint16_t>(input.skills[3]));
        } else {
            skill = std::max(skill, static_cast<std::uint16_t>(input.skills[4]));
        }
        return skill;
    }
    if (action_code < 0x10U) {
        return input.skills[6];
    }
    if (action_code == 0x17U) {
        return input.skills[5];
    }
    return std::nullopt;
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
        std::optional<std::uint16_t> attack_skill;
        if (input.source_mode >= 2U && input.display_state == 0x7F) {
            attack_skill = input.skills[5];
        } else {
            attack_skill = selected_skill_for_action(input, action_code);
        }
        if (!attack_skill.has_value()) {
            result.attack_written = false;
        } else {
            result.attack_value = merge_equipment_action_skill_value(
                attack_base, *attack_skill);
            result.attack_bar = bar_index(result.attack_value);
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

} // namespace shadowrun::ghidra_conversion
