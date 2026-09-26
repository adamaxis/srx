#include "shadowrun_ghidra/entities/entity_action_math_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

std::uint16_t calculate_item18_skill_value(const EntityActionRecord& entity) {
    auto value = static_cast<std::uint16_t>(entity.item18_base);
    for (std::size_t slot = entity.item_codes.size(); slot-- > 0U;) {
        if (entity.item_codes[slot] != 0x18U) {
            continue;
        }
        return static_cast<std::uint8_t>(
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
        return static_cast<std::uint8_t>(
            value + static_cast<std::uint16_t>(entity.item_phases[slot]));
    }
    return value;
}

EntityActionPhaseModifierResult apply_action_item_phase_modifiers(
    const std::uint16_t value,
    const EntityActionRecord& entity,
    const std::uint8_t item_code,
    const std::uint8_t action_index,
    const EntityActionSpecialItemModifierTable* table) {
    EntityActionPhaseModifierResult result{};
    result.value = value;
    for (std::size_t slot = entity.item_codes.size(); slot-- > 0U;) {
        if (entity.item_codes[slot] != item_code) {
            continue;
        }
        result.value = static_cast<std::uint8_t>(
            result.value + static_cast<std::uint16_t>(entity.item_phases[slot]));
        result.phase_found = true;
        break;
    }

    // The source always inspects the eighth possession cell after the phase
    // scan, even when no matching action item was found.
    const auto slot7_item = entity.item_codes[7];
    if (table == nullptr || slot7_item < 0x29U || slot7_item > 0x2BU ||
        action_index >= table->action_offsets.size()) {
        return result;
    }
    const auto row = static_cast<std::size_t>(slot7_item - 0x29U);
    const auto column = table->action_offsets[action_index];
    if (column < 0 || column >= 3) {
        return result;
    }

    result.value = static_cast<std::uint16_t>(
        result.value + static_cast<std::int16_t>(
            table->slot7_item_modifiers[row][column]));
    result.special_item_applied = true;
    if (static_cast<std::int16_t>(result.value) <= 0) {
        result.value = 1U;
    }
    return result;
}

std::uint16_t calculate_ready_entity_action_bonus(
    const std::array<EntityActionRecord, 3>& ready_entities) {
    std::uint16_t bonus = 0U;
    for (const auto& entity : ready_entities) {
        if ((entity.flags_10 & 0x01U) == 0U || entity.class_code > 1U) {
            continue;
        }
        const auto value = calculate_item18_skill_value(entity);
        bonus = static_cast<std::uint16_t>(bonus + value / 4U);
        if ((value % 4U) >= 3U) {
            ++bonus;
        }
    }
    return bonus;
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

EntityActionDynamicModifierResult apply_dynamic_status_modifier(
    std::uint16_t& difficulty,
    const EntityActionDynamicModifierContext& context,
    const EntityActionRollService& roll_service) {
    EntityActionDynamicModifierResult result{};
    std::size_t modifier_index = 0U;
    if ((context.actor_flags_df & 0x01U) != 0U) {
        modifier_index = 0U;
        result.selected_action_table_index = 5U;
    } else if ((context.actor_flags_df & 0x02U) != 0U) {
        modifier_index = 1U;
        result.selected_action_table_index = 12U;
    } else {
        return result;
    }
    result.selected = true;

    const auto* profile =
        context.action_profile_table != nullptr &&
                context.linked_profile_index.has_value()
            ? select_entity_action_profile_row(
                  *context.action_profile_table,
                  *context.linked_profile_index)
            : nullptr;
    if (profile == nullptr) {
        // ROM 0x55828 always returns a selected row for valid source data.
        // Do not substitute caller-provided modifier bytes when that source
        // relationship has not been recovered.
        return result;
    }
    result.action_profile_available = true;
    result.selected_profile_index = *context.linked_profile_index;

    if (context.actor_record_type < 3U) {
        result.ready_roll_count = calculate_ready_entity_action_bonus(
            context.ready_entities);
    }
    const auto attribute = context.attribute_selectors[modifier_index] < 0
                               ? context.actor_attribute_77
                               : context.actor_attribute_7c;
    result.ready_roll_count = static_cast<std::uint16_t>(
        result.ready_roll_count + attribute);

    if (!roll_service) {
        return result;
    }
    result.roll_service_available = true;
    const auto special = modifier_index == 0U
                             ? profile->primary_modifier()
                             : profile->secondary_modifier();
    result.special_successes = roll_service(result.ready_roll_count,
                                             special.difficulty);
    auto remaining = static_cast<int>(special.roll_count) -
                     static_cast<int>(result.special_successes);
    if (remaining <= 0) {
        return result;
    }
    if ((context.actor_flags_df & 0x02U) != 0U) {
        remaining /= 2;
    }
    result.added_difficulty = static_cast<std::uint16_t>(remaining);
    difficulty = static_cast<std::uint16_t>(difficulty + remaining);
    return result;
}

EntityActionPhaseConsumption consume_item13_residual(
    EntityActionRecord& entity,
    const std::uint16_t residual) {
    EntityActionPhaseConsumption result{};
    result.requested = residual;
    result.remaining = residual;
    if (residual == 0U) {
        return result;
    }

    for (std::size_t slot = entity.item_codes.size(); slot-- > 0U;) {
        if (entity.item_codes[slot] != 0x13U) {
            continue;
        }

        const auto phase = entity.item_phases[slot];
        if (result.remaining <= phase) {
            entity.item_phases[slot] = static_cast<std::uint8_t>(
                phase - result.remaining);
            result.remaining = 0U;
            if (entity.item_phases[slot] == 0U) {
                entity.item_codes[slot] = 0U;
                ++result.cleared_slots;
                result.result = EntityActionPhaseConsumptionResult::
                    consumed_exactly;
            } else {
                result.result = EntityActionPhaseConsumptionResult::
                    consumed_partially;
            }
            return result;
        }

        result.remaining = static_cast<std::uint16_t>(
            result.remaining - phase);
        entity.item_codes[slot] = 0U;
        entity.item_phases[slot] = 0U;
        ++result.cleared_slots;
    }

    result.result = EntityActionPhaseConsumptionResult::inventory_exhausted;
    return result;
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

std::uint16_t calculate_action_effect_roll_count(
    const EntityActionRecord& entity) {
    const auto item18_value = calculate_item18_skill_value(entity);
    const auto remaining = select_posture_adjusted_value(
        item18_value, entity.posture, false);
    return static_cast<std::uint8_t>(
        remaining + static_cast<std::uint16_t>(entity.attribute_7c));
}

EntityTargetActionResolution resolve_entity_target_action(
    EntityActionActorState& actor,
    const EntityActionTarget& target,
    const EntityActionCatalogRecord& action,
    const EntityActionDynamicModifierContext* dynamic_context,
    const EntityTargetActionServices& services) {
    EntityTargetActionResolution resolution{};

    // 0x10344..0x10356: an actor already in source behavior state one does
    // not roll. Type-three-and-later records reset equipment to their first
    // possession cell as the original fallback does.
    if (actor.record.mental_meter == 1U) {
        actor.action_result = 0U;
        if (actor.record_type >= 3U) {
            actor.equipped_slot = 0;
            actor.equipped_action_code = actor.first_item_code;
        }
        resolution.outcome = EntityTargetActionOutcome::target_state_gate;
        return resolution;
    }

    // 0x1035A..0x10388: D7=0 selects the posture reduction, then +0x7F,
    // +0x9D, and the first matching (+0x57-1)+0x19 item phase build the
    // initial roll count. The source additions are byte additions.
    const auto skill_value = calculate_item18_skill_value(actor.record);
    auto roll_count = select_posture_adjusted_value(
        skill_value, actor.record.posture, true);
    roll_count = static_cast<std::uint8_t>(
        std::min<std::uint16_t>(roll_count, actor.roll_count_cap));
    roll_count = static_cast<std::uint8_t>(
        roll_count + static_cast<std::uint16_t>(actor.auxiliary_roll_bonus));
    const auto matching_item_code = static_cast<std::uint8_t>(
        static_cast<std::uint16_t>(actor.equipped_action_code) - 1U + 0x19U);
    const auto phase_modifiers = apply_action_item_phase_modifiers(
        roll_count, actor.record, matching_item_code,
        static_cast<std::uint8_t>(actor.equipped_action_code - 1U),
        actor.special_item_modifier_table.has_value()
            ? &*actor.special_item_modifier_table
            : nullptr);
    roll_count = phase_modifiers.value;
    resolution.initial_roll_count = roll_count;

    auto difficulty = static_cast<std::uint16_t>(4U);
    difficulty = apply_target_condition_modifier(
        difficulty, actor.record, target);
    if (dynamic_context != nullptr) {
        const auto dynamic = apply_dynamic_status_modifier(
            difficulty, *dynamic_context, services.roll_service);
        if (dynamic.selected && !dynamic.roll_service_available) {
            resolution.difficulty = difficulty;
            resolution.outcome =
                EntityTargetActionOutcome::roll_service_unavailable;
            return resolution;
        }
    }
    difficulty = add_wound_difficulty_penalty(
        difficulty, actor.record.physical_meter, actor.record.mental_meter);
    difficulty = std::max<std::uint16_t>(difficulty, 2U);
    resolution.difficulty = difficulty;

    if (!services.roll_service) {
        resolution.outcome = EntityTargetActionOutcome::roll_service_unavailable;
        return resolution;
    }
    resolution.successes = services.roll_service(roll_count, difficulty);
    actor.action_result = resolution.successes;
    if (resolution.successes == 0U) {
        resolution.outcome = EntityTargetActionOutcome::roll_failed;
        return resolution;
    }

    // 0x103D8 is an entity-effect service boundary. It is called before the
    // special result writes and before the 0x10660 residual calculation.
    if (services.spawn_entity_effect) {
        services.spawn_entity_effect(actor);
        resolution.effect_service_called = true;
    }
    if (actor.equipped_action_code == 6U ||
        actor.equipped_action_code == 0x0DU) {
        if (services.publish_special_result) {
            services.publish_special_result(resolution.successes,
                                             actor.auxiliary_roll_bonus);
            resolution.special_result_published = true;
        }
    }

    const auto residual = calculate_action_residual(
        actor.equipped_action_code, difficulty,
        calculate_action_effect_roll_count(actor.record), action);
    if (!residual.valid) {
        resolution.outcome = EntityTargetActionOutcome::invalid_action_record;
        return resolution;
    }
    resolution.residual_effect = residual.residual_effect;
    if (residual.residual_effect <= 0) {
        if (actor.record.mental_meter != 0U) {
            resolution.outcome = EntityTargetActionOutcome::
                residual_not_positive;
            return resolution;
        }
        if (!services.advance_behavior_meter) {
            resolution.outcome = EntityTargetActionOutcome::
                behavior_service_unavailable;
            return resolution;
        }
        services.advance_behavior_meter(actor, 1);
        resolution.outcome = EntityTargetActionOutcome::
            behavior_meter_advanced;
        return resolution;
    }

    resolution.phase_consumption = consume_item13_residual(
        actor.record, static_cast<std::uint16_t>(residual.residual_effect));
    switch (resolution.phase_consumption.result) {
    case EntityActionPhaseConsumptionResult::consumed_partially:
        resolution.outcome = EntityTargetActionOutcome::
            residual_consumed_partially;
        return resolution;
    case EntityActionPhaseConsumptionResult::consumed_exactly:
        resolution.outcome = EntityTargetActionOutcome::
            residual_consumed_exactly;
        return resolution;
    case EntityActionPhaseConsumptionResult::inventory_exhausted:
        if (!services.advance_behavior_meter) {
            resolution.outcome = EntityTargetActionOutcome::
                behavior_service_unavailable;
            return resolution;
        }
        services.advance_behavior_meter(
            actor, -static_cast<std::int16_t>(
                       resolution.phase_consumption.remaining));
        if (actor.record.mental_meter == 0U) {
            services.advance_behavior_meter(actor, 1);
            resolution.outcome = EntityTargetActionOutcome::
                behavior_meter_advanced;
        } else {
            resolution.outcome = EntityTargetActionOutcome::
                residual_inventory_exhausted;
        }
        return resolution;
    case EntityActionPhaseConsumptionResult::no_request:
        break;
    }

    resolution.outcome = EntityTargetActionOutcome::residual_not_positive;
    return resolution;
}

}  // namespace shadowrun::ghidra_conversion
