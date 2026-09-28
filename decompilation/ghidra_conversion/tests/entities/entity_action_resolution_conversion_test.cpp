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

    EntityActionActorState typed_actor{};
    EntityActionTarget typed_target{};
    typed_target.armor_index = 7U;
    typed_target.record_type = 3U;
    typed_target.physical_meter = 5U;
    EntityActionResolutionState typed_state{};
    typed_state.armor_mode = 0;
    EntityActionResolutionServices typed_services{};
    typed_services.sample_random_bound =
        [](const std::uint16_t) { return static_cast<std::int16_t>(0); };
    typed_services.apply_defense_modifier =
        [](const std::int16_t value, const EntityActionActorState&,
           const EntityActionTarget&) { return value; };
    typed_services = bind_entity_action_armor_catalog(
        us_armor_catalog(), typed_services);
    const auto typed_primary = resolve_dynamic_entity_action(
        typed_actor, typed_target, 2U, 1U, 10, typed_state, typed_services);
    if (!expect(typed_primary.outcome ==
                    EntityActionResolutionOutcome::target_meter_pending &&
                typed_primary.used_primary_armor &&
                typed_primary.action_value_after_modifiers == 2 &&
                typed_target.physical_meter == 3U)) {
        return 7;
    }

    typed_actor = {};
    typed_target = {};
    typed_target.armor_index = 7U;
    typed_target.record_type = 3U;
    typed_target.physical_meter = 5U;
    typed_state = {};
    typed_state.armor_mode = -1;
    const auto typed_secondary = resolve_dynamic_entity_action(
        typed_actor, typed_target, 2U, 1U, 7, typed_state, typed_services);
    if (!expect(typed_secondary.outcome ==
                    EntityActionResolutionOutcome::target_meter_pending &&
                typed_secondary.used_secondary_armor &&
                typed_secondary.action_value_after_modifiers == 1 &&
                typed_target.physical_meter == 4U)) {
        return 8;
    }

    // ROM 0x55948: active item 5 selects D4540 +0x25 == 1, which NEG.Bs
    // through the typed 0x166AD4 table to an action value of three; +0x26 is
    // the initial difficulty. The opposing target supplies the command,
    // posture, +0x77, and cyberware-bit trial count.
    EntityActionActorState equipment_actor{};
    equipment_actor.equipped_action_code = 5U;
    equipment_actor.action_result = 0U;
    EntityActionTarget equipment_target{};
    equipment_target.record_type = 3U;
    equipment_target.armor_index = 1U;
    equipment_target.physical_meter = 5U;
    equipment_target.attribute_78 = 10U;
    equipment_target.attribute_7b = 10U;
    equipment_target.attribute_7c = 10U;
    equipment_target.posture = 1U;
    equipment_target.attribute_77 = 2U;
    EntityActionResolutionState equipment_state{};
    equipment_state.alternate_meter = 1U;
    equipment_state.armor_mode = -1;
    EntityActionResolutionServices equipment_resolution{};
    equipment_resolution.sample_random_bound =
        [](const std::uint16_t) { return static_cast<std::int16_t>(0); };
    equipment_resolution.apply_defense_modifier =
        [](const std::int16_t value, const EntityActionActorState&,
           const EntityActionTarget&) { return value; };
    equipment_resolution = bind_entity_action_armor_catalog(
        us_armor_catalog(), equipment_resolution);
    const auto equipment_result = resolve_equipment_action_status(
        equipment_actor, equipment_target, equipment_state, {},
        us_item_catalog(),
        {[](const std::uint16_t, const std::uint16_t) {
             return static_cast<std::uint8_t>(0U);
         },
         equipment_resolution});
    if (!expect(equipment_result.outcome ==
                    EquipmentActionStatusOutcome::resolved &&
                equipment_result.source_item_resolved &&
                equipment_result.source_effect_resolved &&
                equipment_result.effect_table_index == 1U &&
                equipment_result.action_effect_value == 3U &&
                equipment_result.initial_difficulty == 10U &&
                equipment_result.effective_difficulty == 10U &&
                equipment_result.target_command_average == 15U &&
                equipment_result.target_posture_remaining == 12U &&
                equipment_result.trial_count == 14U &&
                equipment_result.continuation_invoked &&
                equipment_result.continuation.successes == 0U &&
                equipment_result.continuation.used_primary_armor &&
                equipment_target.physical_meter == 4U &&
                equipment_state.alternate_meter == 0U &&
                equipment_state.armor_mode == 0)) {
        return 9;
    }

    auto mismatched_item_catalog = us_item_catalog();
    mismatched_item_catalog[4].item_code = 6U;
    equipment_actor.action_result = 3U;
    equipment_target.physical_meter = 5U;
    equipment_state = {};
    const auto missing_item = resolve_equipment_action_status(
        equipment_actor, equipment_target, equipment_state, {},
        mismatched_item_catalog, {});
    if (!expect(missing_item.outcome ==
                    EquipmentActionStatusOutcome::source_item_missing &&
                !missing_item.continuation_invoked &&
                equipment_target.physical_meter == 5U)) {
        return 10;
    }

    equipment_actor.equipped_action_code = 41U;
    const auto missing_effect = resolve_equipment_action_status(
        equipment_actor, equipment_target, equipment_state, {},
        us_item_catalog(), {});
    if (!expect(missing_effect.outcome ==
                    EquipmentActionStatusOutcome::source_effect_missing &&
                missing_effect.source_item_resolved &&
                !missing_effect.source_effect_resolved)) {
        return 11;
    }
    return 0;
}
