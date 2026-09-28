#include "shadowrun_ghidra/entities/entity_action_resolution_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {
namespace {

bool positive_source_byte(const std::uint8_t value) {
    return static_cast<std::int8_t>(value) > 0;
}

std::uint8_t add_source_byte(const std::uint8_t left,
                             const std::int16_t right) {
    return static_cast<std::uint8_t>(
        left + static_cast<std::uint8_t>(right));
}

std::int16_t signed_source_byte(const std::uint8_t value) {
    return static_cast<std::int8_t>(value);
}

std::int16_t signed_low_byte(const std::int16_t value) {
    return static_cast<std::int8_t>(static_cast<std::uint8_t>(value));
}

std::int16_t subtract_source_byte(const std::int16_t value,
                                  const std::uint8_t subtraction) {
    const auto low = static_cast<std::uint8_t>(
        static_cast<std::uint8_t>(value) - subtraction);
    return static_cast<std::int16_t>(
        static_cast<std::uint16_t>(value & 0xFF00) | low);
}

std::uint16_t minimum_plus_source_byte(const std::uint16_t component,
                                       const std::uint8_t attribute,
                                       const bool add_attribute) {
    auto value = std::min<std::uint16_t>(component, attribute);
    if (add_attribute) {
        value = static_cast<std::uint16_t>(value + attribute);
    }
    return value == 0U ? 1U : value;
}

std::uint16_t first_cyberware_modifier(const std::uint16_t flags,
                                       const unsigned first_bit,
                                       const unsigned bit_count) {
    for (unsigned offset = 0U; offset < bit_count; ++offset) {
        if ((flags & (static_cast<std::uint16_t>(1U)
                      << (first_bit + offset))) != 0U) {
            return static_cast<std::uint16_t>(offset + 1U);
        }
    }
    return 0U;
}

void advance_reaction_phase_counters(EntityActionResolutionState& state) {
    state.phase_counter = static_cast<std::uint8_t>(
        state.phase_counter - 1U);
    if (static_cast<std::int8_t>(state.phase_counter) >= 0) {
        return;
    }
    state.phase_counter = 0x0FU;
    for (auto& counter : state.phase_roll_counters) {
        counter = static_cast<std::uint8_t>(counter + 1U);
    }
}

} // namespace

EntityActionResolutionServices bind_entity_action_armor_catalog(
    const UsArmorCatalog& catalog,
    EntityActionResolutionServices services) {
    services.subtract_primary_armor =
        [&catalog](const std::int16_t value,
                   const EntityActionTarget& target) {
            return static_cast<std::int16_t>(
                subtract_us_armor_primary_modifier(
                    static_cast<std::uint16_t>(value), target.armor_index,
                    catalog));
        };
    services.subtract_secondary_armor =
        [&catalog](const std::int16_t value,
                   const EntityActionTarget& target) {
            return static_cast<std::int16_t>(
                subtract_us_armor_secondary_modifier(
                    static_cast<std::uint16_t>(value), target.armor_index,
                    catalog));
        };
    return services;
}

EntityActionReactionServices bind_entity_action_armor_catalog(
    const UsArmorCatalog& catalog,
    EntityActionReactionServices services) {
    services.subtract_secondary_armor =
        [&catalog](const std::int16_t value,
                   const EntityActionRecord& target) {
            return static_cast<std::int16_t>(
                subtract_us_armor_secondary_modifier(
                    static_cast<std::uint16_t>(value), target.armor_index,
                    catalog));
        };
    return services;
}

EntityActionDefaultBonusResult resolve_entity_action_with_default_bonus(
    EntityActionActorState& actor,
    EntityActionTarget& target,
    EntityActionResolutionState& state,
    const EntityActionDynamicModifierContext& dynamic_context,
    const EntityActionDefaultBonusServices& services) {
    EntityActionDefaultBonusResult result{};

    // 0x55B1C/0x55B22/0x55B28: the source applies target conditions, the
    // selected dynamic profile, and the actor's wound penalty in that order.
    auto difficulty = apply_target_condition_modifier(
        result.initial_difficulty, actor.record, target);
    const auto dynamic = apply_dynamic_status_modifier(
        difficulty, dynamic_context, services.dynamic_roll_service);
    result.dynamic_modifier_selected = dynamic.selected;
    result.dynamic_modifier_available = dynamic.action_profile_available;
    result.dynamic_roll_service_available = dynamic.roll_service_available;
    if (dynamic.selected && !dynamic.action_profile_available) {
        result.outcome = EntityActionDefaultBonusOutcome::
            dynamic_modifier_data_unavailable;
        return result;
    }
    if (dynamic.selected && !dynamic.roll_service_available) {
        result.outcome = EntityActionDefaultBonusOutcome::
            dynamic_modifier_roll_service_unavailable;
        return result;
    }
    difficulty = add_wound_difficulty_penalty(
        difficulty, actor.record.physical_meter, actor.record.mental_meter);
    difficulty = std::max<std::uint16_t>(difficulty, 2U);
    result.effective_difficulty = difficulty;

    // 0x55B3A..0x55B40: D6 is a fixed six, so the source always performs at
    // least one bounded roll even though the host keeps the count typed.
    if (!services.sample_random_bound) {
        result.outcome = EntityActionDefaultBonusOutcome::
            random_service_unavailable;
        return result;
    }
    result.initial_successes = count_thresholded_random_rolls(
        result.initial_trial_count, static_cast<std::int16_t>(difficulty),
        services.sample_random_bound);
    actor.action_result = result.initial_successes;
    actor.auxiliary_roll_bonus = 6U;

    // 0x55B54..0x55B64: D5 is reset to two, the target's +0x77 becomes the
    // continuation trial count, and only low-type targets receive the ready
    // animation refresh before the shared continuation.
    result.continuation_trial_count = target.attribute_77;
    if (target.record_type < 3U && services.refresh_target_ready_animations) {
        services.refresh_target_ready_animations(target);
        result.target_animation_refresh_called = true;
    }
    state.armor_mode = 1;
    const EntityActionResolutionServices continuation_services{
        services.sample_random_bound,
        {},
        {},
        services.apply_defense_modifier,
        services.advance_behavior_meter,
        services.advance_delay_meter};
    result.continuation = resolve_dynamic_entity_action(
        actor, target, 6U, result.continuation_trial_count, 2, state,
        continuation_services);
    result.outcome = EntityActionDefaultBonusOutcome::resolved;
    return result;
}

EntityActionReactionResult advance_entity_action_resolution(
    EntityActionReactionParticipant& actor,
    EntityActionReactionParticipant& target,
    EntityActionResolutionState& state,
    const EntityActionDynamicModifierContext& actor_dynamic_context,
    const EntityActionDynamicModifierContext& target_dynamic_context,
    const EntityActionReactionServices& services) {
    EntityActionReactionResult result{};

    // 0x55C76..0x55C96: the actor contributes the posture reduction capped
    // by +0x85, then adds +0x85 a second time. The zero correction is before
    // the first thresholded roll.
    result.initial_command_average =
        calculate_equipment_command_average(actor.record);
    result.initial_posture_reduction = select_posture_adjusted_value(
        result.initial_command_average, actor.record.posture, true);
    result.initial_trial_count = minimum_plus_source_byte(
        result.initial_posture_reduction, actor.record.attribute_85, true);

    auto difficulty = apply_target_condition_modifier(
        4U, actor.record, target.record);
    const auto actor_dynamic = apply_dynamic_status_modifier(
        difficulty, actor_dynamic_context, services.dynamic_roll_service);
    if (actor_dynamic.selected && !actor_dynamic.action_profile_available) {
        result.outcome = EntityActionReactionOutcome::
            dynamic_modifier_data_unavailable;
        return result;
    }
    if (actor_dynamic.selected && !actor_dynamic.roll_service_available) {
        result.outcome = EntityActionReactionOutcome::
            dynamic_modifier_roll_service_unavailable;
        return result;
    }
    difficulty = std::max<std::uint16_t>(difficulty, 2U);
    result.initial_difficulty = difficulty;

    if (!services.sample_random_bound) {
        result.outcome = EntityActionReactionOutcome::
            random_service_unavailable;
        return result;
    }
    result.first_successes = count_thresholded_random_rolls(
        result.initial_trial_count, static_cast<std::int16_t>(difficulty),
        services.sample_random_bound);
    if (result.first_successes == 0U) {
        result.outcome = EntityActionReactionOutcome::first_roll_failed;
        return result;
    }

    // 0x55CB4..0x55CC8: a successful first result becomes the low-byte
    // action value, receives the fixed three-point addition, then passes
    // through secondary armor and the defense modifier.
    auto remaining = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(result.first_successes) + 3U);
    if (!services.subtract_secondary_armor) {
        result.outcome = EntityActionReactionOutcome::
            secondary_armor_service_unavailable;
        return result;
    }
    result.used_secondary_armor = true;
    remaining = services.subtract_secondary_armor(remaining,
                                                  target.record);
    if (signed_low_byte(remaining) <= 0) {
        result.value_after_first_modifiers = remaining;
        result.outcome = EntityActionReactionOutcome::
            first_armor_threshold_failed;
        return result;
    }
    if (!services.apply_defense_modifier) {
        result.value_after_first_modifiers = remaining;
        result.outcome = EntityActionReactionOutcome::
            defense_service_unavailable;
        return result;
    }
    remaining = services.apply_defense_modifier(remaining, actor.record,
                                                target.record);
    result.value_after_first_modifiers = remaining;
    if (signed_low_byte(remaining) <= 0) {
        result.outcome = EntityActionReactionOutcome::
            first_defense_threshold_failed;
        return result;
    }

    // 0x55CD0..0x55D0A: swap the two decoded records for the peer pass. The
    // peer's +0x85 is only doubled when its action code is zero.
    result.peer_command_average =
        calculate_equipment_command_average(target.record);
    result.peer_posture_reduction = select_posture_adjusted_value(
        result.peer_command_average, target.record.posture, true);
    result.peer_trial_count = minimum_plus_source_byte(
        result.peer_posture_reduction, target.record.attribute_85,
        target.equipped_action_code == 0U);
    auto peer_difficulty = apply_target_condition_modifier(
        4U, target.record, actor.record);
    const auto target_dynamic = apply_dynamic_status_modifier(
        peer_difficulty, target_dynamic_context,
        services.dynamic_roll_service);
    if (target_dynamic.selected && !target_dynamic.action_profile_available) {
        result.outcome = EntityActionReactionOutcome::
            dynamic_modifier_data_unavailable;
        return result;
    }
    if (target_dynamic.selected && !target_dynamic.roll_service_available) {
        result.outcome = EntityActionReactionOutcome::
            dynamic_modifier_roll_service_unavailable;
        return result;
    }
    peer_difficulty = std::max<std::uint16_t>(peer_difficulty, 2U);
    result.peer_difficulty = peer_difficulty;
    result.second_successes = count_thresholded_random_rolls(
        result.peer_trial_count, static_cast<std::int16_t>(peer_difficulty),
        services.sample_random_bound);
    remaining = subtract_source_byte(
        remaining, result.second_successes);
    if (signed_low_byte(remaining) <= 0) {
        result.remaining_value = remaining;
        result.outcome = EntityActionReactionOutcome::second_roll_failed;
        return result;
    }

    // 0x55D26..0x55D86: actor +0x79 selects the final difficulty, while the
    // actor's first set bit in cyberware flags 5..8 adds one through four to
    // it. The peer's +0x77 and first set bit in flags 9..11 form the final
    // trial count.
    auto final_difficulty = static_cast<std::uint16_t>(
        actor.record.attribute_79 +
        first_cyberware_modifier(actor.record.cyberware_flags, 5U, 4U));
    auto final_trial_count = static_cast<std::uint16_t>(
        static_cast<std::uint8_t>(result.peer_trial_count +
                                  target.record.attribute_77));
    final_trial_count = static_cast<std::uint16_t>(
        final_trial_count +
        first_cyberware_modifier(target.record.cyberware_flags, 9U, 3U));
    final_difficulty = std::max<std::uint16_t>(final_difficulty, 2U);
    result.final_trial_count = final_trial_count;
    result.final_difficulty = final_difficulty;
    result.final_successes = count_thresholded_random_rolls(
        final_trial_count, static_cast<std::int16_t>(final_difficulty),
        services.sample_random_bound);
    remaining = subtract_source_byte(remaining, result.final_successes);
    if (signed_low_byte(remaining) <= 0) {
        result.remaining_value = remaining;
        result.outcome = EntityActionReactionOutcome::final_roll_failed;
        return result;
    }

    // 0x55DA2..0x55DBC: bit three selects direct physical damage; otherwise
    // bit two selects physical damage after the source two-point byte penalty;
    // neither bit selects the mental-meter branch.
    const auto has_physical_modifier =
        (actor.record.cyberware_flags & 0x0008U) != 0U;
    const auto has_physical_penalty =
        (actor.record.cyberware_flags & 0x0004U) != 0U;
    if (!has_physical_modifier && has_physical_penalty) {
        remaining = subtract_source_byte(remaining, 2U);
        if (signed_low_byte(remaining) <= 0) {
            result.remaining_value = remaining;
            result.outcome = EntityActionReactionOutcome::physical_penalty_failed;
            return result;
        }
    }
    result.remaining_value = remaining;
    result.meter_delta = static_cast<std::int16_t>(-remaining);
    result.used_physical_meter = has_physical_modifier || has_physical_penalty;
    result.used_mental_meter = !result.used_physical_meter;

    if (target.record_type < 3U) {
        if (state.world_mode == 3U) {
            result.outcome = EntityActionReactionOutcome::world_meter_gate;
            return result;
        }
        if (result.used_physical_meter) {
            if (!services.advance_delay_meter) {
                result.outcome = EntityActionReactionOutcome::
                    delay_service_unavailable;
                return result;
            }
            services.advance_delay_meter(target);
            result.outcome = EntityActionReactionOutcome::
                physical_meter_pending;
            return result;
        }
        if (!services.advance_behavior_meter) {
            result.outcome = EntityActionReactionOutcome::
                behavior_service_unavailable;
            return result;
        }
        services.advance_behavior_meter(target, result.meter_delta);
        result.outcome = EntityActionReactionOutcome::mental_meter_pending;
        return result;
    }

    auto& meter = result.used_physical_meter ? target.record.physical_meter
                                             : target.record.mental_meter;
    if (target.record.race_or_state < 0) {
        result.meter_delta = -1;
    }
    meter = add_source_byte(meter, result.meter_delta);
    if (positive_source_byte(meter)) {
        result.outcome = result.used_physical_meter
                             ? EntityActionReactionOutcome::physical_meter_pending
                             : EntityActionReactionOutcome::mental_meter_pending;
        return result;
    }
    meter = 0U;
    advance_reaction_phase_counters(state);
    result.counters_advanced = true;
    result.outcome = EntityActionReactionOutcome::meter_exhausted;
    return result;
}

EntityActionResolutionResult resolve_dynamic_entity_action(
    EntityActionActorState& actor,
    EntityActionTarget& target,
    const std::uint16_t difficulty,
    const std::uint16_t trial_count,
    const std::int16_t action_value,
    EntityActionResolutionState& state,
    const EntityActionResolutionServices& services) {
    EntityActionResolutionResult result{};
    result.difficulty = std::max<std::uint16_t>(difficulty, 2U);
    result.trial_count = trial_count;
    result.previous_action_result = actor.action_result;
    result.action_value_before_modifiers = action_value;

    // 0x559C8: D6 trials at the already-clamped D4 threshold.
    if (!services.sample_random_bound) {
        return result;
    }
    result.successes = count_thresholded_random_rolls(
        trial_count, static_cast<std::int16_t>(result.difficulty),
        services.sample_random_bound);

    // 0x559CE..0x559F0 compares the new threshold result with the actor's
    // previous +0x9F byte. A negative byte delta uses ADD.B/BGT; a
    // nonnegative delta uses ADD.W and clamps values above ten to ten.
    const auto delta_byte = static_cast<std::uint8_t>(
        actor.action_result - result.successes);
    const auto signed_delta = signed_source_byte(delta_byte);
    auto adjusted_value = action_value;
    if (signed_delta < 0) {
        const auto adjusted_byte = add_source_byte(
            static_cast<std::uint8_t>(action_value), signed_delta);
        adjusted_value = signed_source_byte(adjusted_byte);
        if (!positive_source_byte(adjusted_byte)) {
            result.action_value_after_modifiers = adjusted_value;
            result.outcome = EntityActionResolutionOutcome::
                action_value_nonpositive;
            return result;
        }
    } else {
        adjusted_value = static_cast<std::int16_t>(
            action_value + static_cast<std::int16_t>(
                               static_cast<std::uint8_t>(delta_byte)));
        if (adjusted_value >= 0x0BU) {
            adjusted_value = 0x0AU;
        }
    }
    result.action_value_after_modifiers = adjusted_value;

    // 0x559F4..0x5A1A: mode -1 selects secondary armor, mode 0 selects
    // primary armor, positive modes skip both; defense reduction follows.
    if (state.armor_mode == 0) {
        if (!services.subtract_primary_armor) {
            result.outcome = EntityActionResolutionOutcome::
                armor_modifier_unavailable;
            return result;
        }
        result.used_primary_armor = true;
        adjusted_value = services.subtract_primary_armor(adjusted_value,
                                                          target);
        if (adjusted_value <= 0) {
            result.action_value_after_modifiers = adjusted_value;
            result.outcome = EntityActionResolutionOutcome::
                armor_threshold_failed;
            return result;
        }
    } else if (state.armor_mode < 0) {
        if (!services.subtract_secondary_armor) {
            result.outcome = EntityActionResolutionOutcome::
                armor_modifier_unavailable;
            return result;
        }
        result.used_secondary_armor = true;
        adjusted_value = services.subtract_secondary_armor(adjusted_value,
                                                            target);
        if (adjusted_value <= 0) {
            result.action_value_after_modifiers = adjusted_value;
            result.outcome = EntityActionResolutionOutcome::
                armor_threshold_failed;
            return result;
        }
    }

    if (!services.apply_defense_modifier) {
        result.outcome = EntityActionResolutionOutcome::
            defense_modifier_unavailable;
        return result;
    }
    adjusted_value = services.apply_defense_modifier(
        adjusted_value, actor, target);
    result.action_value_after_modifiers = adjusted_value;
    if (adjusted_value <= 0) {
        result.outcome = EntityActionResolutionOutcome::
            defense_threshold_failed;
        return result;
    }

    // 0x55A1E..0x55A60: records below type three use the delay/behavior
    // service rather than the target's physical/mental meters. FFE186==3
    // blocks that service without inventing a state transition.
    result.meter_delta = static_cast<std::int16_t>(-adjusted_value);
    if (target.record_type < 3U) {
        if (state.world_mode == 3U) {
            result.outcome = EntityActionResolutionOutcome::world_meter_gate;
            return result;
        }
        if (state.alternate_meter != 0U) {
            state.alternate_meter = 0U;
            if (!services.advance_behavior_meter) {
                result.outcome = EntityActionResolutionOutcome::
                    behavior_service_unavailable;
                return result;
            }
            services.advance_behavior_meter(target, 1);
            result.outcome = EntityActionResolutionOutcome::
                behavior_meter_advanced;
            return result;
        }
        if (!services.advance_delay_meter) {
            result.outcome = EntityActionResolutionOutcome::
                delay_service_unavailable;
            return result;
        }
        services.advance_delay_meter(target);
        result.outcome = EntityActionResolutionOutcome::delay_meter_advanced;
        return result;
    }

    // 0x55A62..0x55A9E: a signed-negative target state reduces the meter by
    // one regardless of the action value. Otherwise the full negative value
    // is added to physical or mental state according to FFF0CC.
    if (target.race_or_state < 0) {
        result.meter_delta = -1;
    }
    const auto meter_delta = result.meter_delta;
    if (state.alternate_meter != 0U) {
        state.alternate_meter = 0U;
        result.used_mental_meter = true;
        const auto updated = add_source_byte(target.mental_meter, meter_delta);
        target.mental_meter = updated;
        if (positive_source_byte(updated)) {
            result.outcome = EntityActionResolutionOutcome::target_meter_pending;
            return result;
        }
        target.mental_meter = 0U;
    } else {
        const auto updated = add_source_byte(target.physical_meter,
                                             meter_delta);
        target.physical_meter = updated;
        if (positive_source_byte(updated)) {
            result.outcome = EntityActionResolutionOutcome::target_meter_pending;
            return result;
        }
        target.physical_meter = 0U;
    }

    // 0x55A9E..0x55B0E: the source's class-B story counter, phase rollover,
    // and class-0x13 counter are all byte/word operations with explicit
    // source clamps.
    if (!state.story_counter_gate && target.class_code == 0x42U) {
        ++state.class_b_counter;
        if (state.class_b_counter >= 0x15U) {
            state.class_b_counter = 0x14U;
        }
    }
    state.phase_counter = static_cast<std::uint8_t>(
        state.phase_counter - 1U);
    if (static_cast<std::int8_t>(state.phase_counter) < 0) {
        state.phase_counter = 0x0FU;
        for (auto& counter : state.phase_roll_counters) {
            counter = static_cast<std::uint8_t>(counter + 1U);
        }
    }
    if (target.class_code == 0x13U) {
        state.class13_counter = static_cast<std::uint8_t>(
            state.class13_counter + 1U);
        if (state.class13_counter >= 9U) {
            state.class13_counter = 8U;
        }
    }
    result.counters_advanced = true;
    result.outcome = EntityActionResolutionOutcome::target_meter_exhausted;
    return result;
}

} // namespace shadowrun::ghidra_conversion
