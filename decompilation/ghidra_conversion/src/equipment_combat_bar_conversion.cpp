#include "shadowrun_ghidra/equipment_combat_bar_conversion.hpp"

#include <algorithm>

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

std::uint16_t command_average(const EquipmentCombatInput& input) {
    const auto attribute_average =
        (static_cast<unsigned>(input.attributes[1]) +
         static_cast<unsigned>(input.attributes[4]) +
         static_cast<unsigned>(input.attributes[5])) /
        2U;
    return static_cast<std::uint16_t>(
        attribute_average + inventory_phase_for_code(input, 0x28U));
}

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

std::uint16_t merge_skill(const std::uint16_t current,
                          const std::uint16_t skill) {
    const auto lower = std::min(current, skill);
    const auto result = static_cast<std::uint16_t>(lower + skill);
    return result == 0U ? 1U : result;
}

std::uint8_t bar_index(const std::uint16_t value) {
    return static_cast<std::uint8_t>(
        std::min<unsigned>(22U, value == 0U ? 0U : value - 1U));
}

std::uint16_t selected_skill(const EquipmentCombatInput& input) {
    if (input.display_state == 0x7F) {
        return input.skills[5];
    }
    if (input.display_state < 0) {
        return 0U;
    }

    if (input.active_item_code < 0x0DU) {
        auto skill = static_cast<std::uint16_t>(input.skills[1]);
        if (input.active_item_code < 0x08U) {
            skill = std::max(skill, static_cast<std::uint16_t>(input.skills[2]));
        } else if (input.active_item_code < 0x0BU) {
            skill = std::max(skill, static_cast<std::uint16_t>(input.skills[3]));
        } else {
            skill = std::max(skill, static_cast<std::uint16_t>(input.skills[4]));
        }
        return skill;
    }
    if (input.active_item_code < 0x10U) {
        return input.skills[6];
    }
    if (input.active_item_code == 0x17U) {
        return input.skills[5];
    }
    return 0U;
}

} // namespace

EquipmentCombatBarResult compute_equipment_combat_bars(
    const EquipmentCombatInput& input) {
    EquipmentCombatBarResult result{};
    result.command_average = command_average(input);

    if (input.display_state < 0 && input.display_state != 0x7F) {
        result.attack_written = false;
        result.defense_written = false;
        return result;
    }

    const auto attack_base = posture_value(result.command_average,
                                           input.posture, false);
    const auto attack_skill = selected_skill(input);
    if (input.display_state >= 0 || input.display_state == 0x7F) {
        result.attack_value = merge_skill(attack_base, attack_skill);
        result.attack_bar = bar_index(result.attack_value);
    }

    const auto selected_item =
        input.selected_inventory_slot < input.inventory_item_codes.size()
            ? input.inventory_item_codes[input.selected_inventory_slot]
            : 0U;
    if (selected_item >= 0x10U && selected_item != 0x17U) {
        result.defense_written = false;
        return result;
    }

    auto defense_value = posture_value(result.command_average, input.posture,
                                       true);
    defense_value = static_cast<std::uint16_t>(
        defense_value + input.attributes[0]);
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
