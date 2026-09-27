#include "shadowrun_ghidra/entities/entity_action_resolution_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityActionActorState actor{};
    actor.action_result = 2U;
    EntityActionTarget target{};
    target.record_type = 3U;
    target.physical_meter = 5U;
    EntityActionResolutionState state{};
    state.armor_mode = -1;
    const EntityActionResolutionServices unavailable{};
    const auto missing = resolve_dynamic_entity_action(
        actor, target, 4U, 3U, 2, state, unavailable);
    if (!expect(missing.outcome ==
                    EntityActionResolutionOutcome::random_service_unavailable &&
                target.physical_meter == 5U)) {
        return 1;
    }

    std::vector<std::int16_t> rolls(3U, 0);
    std::size_t roll_cursor = 0U;
    state = {};
    state.armor_mode = -1;
    const EntityActionResolutionServices services{
        [&rolls, &roll_cursor](const std::uint16_t bound) {
            if (bound != 6U || roll_cursor >= rolls.size()) {
                return static_cast<std::int16_t>(0);
            }
            return rolls[roll_cursor++];
        },
        {},
        [](const std::int16_t value, const EntityActionTarget&) {
            return static_cast<std::int16_t>(value - 1);
        },
        [](const std::int16_t value, const EntityActionActorState&,
           const EntityActionTarget&) {
            return static_cast<std::int16_t>(value - 1);
        },
        {},
        {}};
    const auto pending = resolve_dynamic_entity_action(
        actor, target, 4U, 3U, 2, state, services);
    if (!expect(pending.outcome ==
                    EntityActionResolutionOutcome::target_meter_pending &&
                pending.successes == 0U &&
                pending.previous_action_result == 2U &&
                pending.action_value_after_modifiers == 2 &&
                pending.used_secondary_armor &&
                !pending.used_primary_armor && target.physical_meter == 3U)) {
        return 2;
    }

    actor.action_result = 0U;
    target = {};
    target.record_type = 3U;
    target.physical_meter = 1U;
    target.class_code = 0x13U;
    state = {};
    state.armor_mode = 1;
    state.phase_counter = 0U;
    state.class13_counter = 8U;
    const EntityActionResolutionServices direct_services{
        [](const std::uint16_t) { return static_cast<std::int16_t>(0); },
        {},
        {},
        [](const std::int16_t value, const EntityActionActorState&,
           const EntityActionTarget&) { return value; },
        {},
        {}};
    const auto exhausted = resolve_dynamic_entity_action(
        actor, target, 2U, 1U, 2, state, direct_services);
    if (!expect(exhausted.outcome ==
                    EntityActionResolutionOutcome::target_meter_exhausted &&
                target.physical_meter == 0U && exhausted.counters_advanced &&
                state.phase_counter == 0x0FU &&
                state.phase_roll_counters[0] == 1U &&
                state.class13_counter == 8U)) {
        return 3;
    }

    target = {};
    target.record_type = 2U;
    state = {};
    state.armor_mode = 1;
    bool delay_called = false;
    const EntityActionResolutionServices delay_services{
        [](const std::uint16_t) { return static_cast<std::int16_t>(0); },
        {},
        {},
        [](const std::int16_t value, const EntityActionActorState&,
           const EntityActionTarget&) { return value; },
        {},
        [&delay_called](EntityActionTarget&) { delay_called = true; }};
    const auto delayed = resolve_dynamic_entity_action(
        actor, target, 2U, 1U, 1, state, delay_services);
    if (!expect(delayed.outcome ==
                    EntityActionResolutionOutcome::delay_meter_advanced &&
                delay_called)) {
        return 4;
    }

    state.alternate_meter = 1U;
    bool behavior_called = false;
    const EntityActionResolutionServices behavior_services{
        [](const std::uint16_t) { return static_cast<std::int16_t>(0); },
        {},
        {},
        [](const std::int16_t value, const EntityActionActorState&,
           const EntityActionTarget&) { return value; },
        [&behavior_called](EntityActionTarget&, const std::int16_t delta) {
            behavior_called = delta == 1;
        },
        {}};
    const auto behaved = resolve_dynamic_entity_action(
        actor, target, 2U, 1U, 1, state, behavior_services);
    if (!expect(behaved.outcome ==
                    EntityActionResolutionOutcome::behavior_meter_advanced &&
                state.alternate_meter == 0U && behavior_called)) {
        return 5;
    }

    state.world_mode = 3U;
    const auto blocked = resolve_dynamic_entity_action(
        actor, target, 2U, 1U, 1, state, behavior_services);
    if (!expect(blocked.outcome ==
                EntityActionResolutionOutcome::world_meter_gate)) {
        return 6;
    }
    return 0;
}
