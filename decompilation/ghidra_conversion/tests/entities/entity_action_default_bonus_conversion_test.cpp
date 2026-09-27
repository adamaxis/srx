#include "shadowrun_ghidra/entities/entity_action_resolution_conversion.hpp"

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityActionActorState actor{};
    actor.record.physical_meter = 5U;
    actor.record.mental_meter = 6U;
    EntityActionTarget target{};
    target.record_type = 2U;
    target.attribute_77 = 1U;
    EntityActionResolutionState state{};
    EntityActionDynamicModifierContext dynamic_context{};

    int random_calls = 0;
    bool target_animation_refreshed = false;
    bool delay_advanced = false;
    EntityActionDefaultBonusServices services{};
    services.sample_random_bound = [&random_calls](const std::uint16_t bound) {
        if (bound != 6U) {
            return static_cast<std::int16_t>(0);
        }
        ++random_calls;
        return static_cast<std::int16_t>(0);
    };
    services.apply_defense_modifier =
        [](const std::int16_t value, const EntityActionActorState&,
           const EntityActionTarget&) { return value; };
    services.refresh_target_ready_animations =
        [&target_animation_refreshed](EntityActionTarget&) {
            target_animation_refreshed = true;
        };
    services.advance_delay_meter = [&delay_advanced](EntityActionTarget&) {
        delay_advanced = true;
    };

    const auto resolved = resolve_entity_action_with_default_bonus(
        actor, target, state, dynamic_context, services);
    if (resolved.outcome != EntityActionDefaultBonusOutcome::resolved ||
        resolved.initial_difficulty != 4U ||
        resolved.effective_difficulty != 6U ||
        resolved.initial_trial_count != 6U ||
        resolved.continuation_trial_count != 1U ||
        resolved.initial_successes != 0U || actor.action_result != 0U ||
        actor.auxiliary_roll_bonus != 6U || state.armor_mode != 1 ||
        !resolved.target_animation_refresh_called ||
        !target_animation_refreshed || !delay_advanced ||
        resolved.continuation.outcome !=
            EntityActionResolutionOutcome::delay_meter_advanced ||
        random_calls != 7) {
        return 1;
    }

    dynamic_context.actor_flags_df = 1U;
    actor.action_result = 3U;
    const auto missing_profile = resolve_entity_action_with_default_bonus(
        actor, target, state, dynamic_context, services);
    if (missing_profile.outcome !=
            EntityActionDefaultBonusOutcome::
                dynamic_modifier_data_unavailable ||
        actor.action_result != 3U || random_calls != 7) {
        return 2;
    }
    return 0;
}
