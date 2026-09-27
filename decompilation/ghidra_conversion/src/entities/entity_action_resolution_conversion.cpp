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

} // namespace

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
