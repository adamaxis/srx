#include "shadowrun_ghidra/entity_action_math_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

std::uint16_t calculate_item18_skill_value(const EntityActionRecord& entity) {
    auto value = static_cast<std::uint16_t>(entity.item18_base);
    for (std::size_t slot = entity.item_codes.size(); slot-- > 0U;) {
        if (entity.item_codes[slot] != 0x18U) {
            continue;
        }
        return static_cast<std::uint16_t>(
            value + static_cast<std::uint16_t>(entity.item_phases[slot]));
    }
    return value;
}

std::uint16_t select_posture_adjusted_value(
    const std::uint16_t input,
    const std::uint8_t posture,
    const bool select_reduced_component) {
    // The 68000 performs the multiply before the DIVU. Host arithmetic keeps
    // that intermediate wide and narrows only at the source word result.
    const auto reduction = static_cast<std::uint16_t>(
        (static_cast<std::uint32_t>(input) * posture * 25U) / 100U);
    const auto remaining = static_cast<std::uint16_t>(input - reduction);
    return select_reduced_component ? reduction : remaining;
}

std::uint16_t add_first_matching_item_phase(
    const std::uint16_t value,
    const EntityActionRecord& entity,
    const std::uint8_t item_code) {
    for (std::size_t slot = entity.item_codes.size(); slot-- > 0U;) {
        if (entity.item_codes[slot] != item_code) {
            continue;
        }
        return static_cast<std::uint16_t>(
            value + static_cast<std::uint16_t>(entity.item_phases[slot]));
    }
    return value;
}

std::uint16_t add_wound_difficulty_penalty(
    std::uint16_t difficulty,
    const std::uint8_t physical_meter,
    const std::uint8_t mental_meter) {
    const auto add_meter_penalty = [&difficulty](const std::uint8_t meter) {
        if (meter >= 8U) {
            return;
        }
        difficulty = static_cast<std::uint16_t>(
            difficulty + (meter >= 5U ? 1U : 2U));
    };
    add_meter_penalty(physical_meter);
    add_meter_penalty(mental_meter);
    return difficulty;
}

std::uint16_t apply_target_condition_modifier(
    std::uint16_t difficulty,
    const EntityActionRecord& attacker,
    const EntityActionTarget& target) {
    const auto has_cyberware_modifier =
        (attacker.cyberware_flags & 0x0002U) != 0U;
    if (has_cyberware_modifier) {
        --difficulty;
    }

    if (target.class_code == 0x16U) {
        return static_cast<std::uint16_t>(difficulty + 2U);
    }
    if (has_cyberware_modifier || attacker.race_or_state == 0x04U ||
        attacker.race_or_state == 0x02U ||
        (target.flags & 0x80U) == 0U) {
        return difficulty;
    }
    return static_cast<std::uint16_t>(difficulty + 4U);
}

EntityActionResidual calculate_action_residual(
    const std::uint8_t action_code,
    const std::uint16_t difficulty,
    const std::uint16_t roll_count,
    const EntityActionCatalogRecord& action) {
    if (action_code == 0U) {
        return {};
    }

    // 0x10678: LSR.W then add one when the discarded bit was set, which is
    // ceil(difficulty / 2), followed by the row's signed-byte modifier.
    auto effective_difficulty = static_cast<std::uint16_t>(
        (difficulty + 1U) / 2U + action.difficulty_modifier);
    effective_difficulty = std::max<std::uint16_t>(effective_difficulty, 2U);

    auto percent_steps = effective_difficulty >= 7U
                             ? 10U
                             : static_cast<unsigned>(7U -
                                                      effective_difficulty) *
                                   20U;
    const auto scaled_product =
        static_cast<std::uint32_t>(roll_count) * percent_steps;
    auto adjusted_roll_count = scaled_product / 100U;
    if ((scaled_product % 100U) >= 50U) {
        ++adjusted_roll_count;
    }
    adjusted_roll_count /= 2U;

    const auto residual = static_cast<std::int32_t>(action.base_effect) -
                          static_cast<std::int32_t>(adjusted_roll_count);
    return {true, effective_difficulty,
            static_cast<std::uint16_t>(adjusted_roll_count),
            static_cast<std::int16_t>(residual)};
}

}  // namespace shadowrun::ghidra_conversion
