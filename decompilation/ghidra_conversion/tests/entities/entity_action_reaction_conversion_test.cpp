#include "shadowrun_ghidra/entities/entity_action_resolution_conversion.hpp"

#include <vector>

namespace {

shadowrun::ghidra_conversion::EntityActionReactionServices make_services(
    std::vector<std::int16_t>& rolls,
    std::size_t& cursor) {
    using namespace shadowrun::ghidra_conversion;
    EntityActionReactionServices services{};
    services.sample_random_bound = [&rolls, &cursor](const std::uint16_t bound) {
        if (bound != 6U || cursor >= rolls.size()) {
            return static_cast<std::int16_t>(0);
        }
        return rolls[cursor++];
    };
    services.subtract_secondary_armor =
        [](const std::int16_t value, const EntityActionRecord&) {
            return static_cast<std::int16_t>(value - 1);
        };
    services.apply_defense_modifier =
        [](const std::int16_t value, const EntityActionRecord&,
           const EntityActionRecord&) {
            return static_cast<std::int16_t>(value - 1);
        };
    return services;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityActionReactionParticipant actor{};
    actor.record.attribute_78 = 2U;
    actor.record.attribute_79 = 2U;
    actor.record.attribute_85 = 1U;

    EntityActionReactionParticipant target{};
    target.record.attribute_78 = 2U;
    target.record.attribute_85 = 1U;
    target.record_type = 3U;
    target.record.mental_meter = 5U;
    target.equipped_action_code = 1U;

    EntityActionDynamicModifierContext actor_context{};
    EntityActionDynamicModifierContext target_context{};
    EntityActionResolutionState state{};
    std::vector<std::int16_t> rolls{3, 0, 0};
    std::size_t cursor = 0U;
    auto services = make_services(rolls, cursor);

    const auto pending = advance_entity_action_resolution(
        actor, target, state, actor_context, target_context, services);
    if (pending.outcome !=
            EntityActionReactionOutcome::mental_meter_pending ||
        pending.initial_command_average != 1U ||
        pending.initial_posture_reduction != 0U ||
        pending.initial_trial_count != 1U ||
        pending.initial_difficulty != 4U || pending.first_successes != 1U ||
        pending.value_after_first_modifiers != 2 ||
        pending.peer_command_average != 1U || pending.peer_trial_count != 1U ||
        pending.peer_difficulty != 4U || pending.second_successes != 0U ||
        pending.final_trial_count != 1U || pending.final_difficulty != 2U ||
        pending.final_successes != 0U || pending.remaining_value != 2 ||
        pending.meter_delta != -2 || !pending.used_mental_meter ||
        pending.used_physical_meter || target.record.mental_meter != 3U ||
        cursor != 3U) {
        return 1;
    }

    target.record.mental_meter = 1U;
    state = {};
    cursor = 0U;
    const auto exhausted = advance_entity_action_resolution(
        actor, target, state, actor_context, target_context, services);
    if (exhausted.outcome != EntityActionReactionOutcome::meter_exhausted ||
        target.record.mental_meter != 0U || !exhausted.counters_advanced ||
        state.phase_counter != 0x0FU ||
        state.phase_roll_counters[0] != 1U ||
        state.phase_roll_counters[1] != 1U ||
        state.phase_roll_counters[2] != 1U) {
        return 2;
    }

    actor.record.cyberware_flags = 0x0008U;
    target.record.physical_meter = 5U;
    target.record.mental_meter = 0U;
    state = {};
    cursor = 0U;
    const auto physical = advance_entity_action_resolution(
        actor, target, state, actor_context, target_context, services);
    if (physical.outcome !=
            EntityActionReactionOutcome::physical_meter_pending ||
        !physical.used_physical_meter || target.record.physical_meter != 3U) {
        return 3;
    }

    actor.record.cyberware_flags = 0U;
    target.record.physical_meter = 0U;
    target.record.mental_meter = 5U;
    target.record.race_or_state = -1;
    state = {};
    cursor = 0U;
    const auto signed_state = advance_entity_action_resolution(
        actor, target, state, actor_context, target_context, services);
    if (signed_state.outcome !=
            EntityActionReactionOutcome::mental_meter_pending ||
        signed_state.meter_delta != -1 ||
        target.record.mental_meter != 4U) {
        return 4;
    }
    return 0;
}
