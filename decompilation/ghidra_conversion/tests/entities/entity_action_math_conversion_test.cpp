#include "shadowrun_ghidra/entities/entity_action_math_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) { return condition; }

}  // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<std::uint8_t> profile_bytes(
        kEntityActionProfileCount * kEntityActionProfileRowSize, 0U);
    const auto profile_offset = 2U * kEntityActionProfileRowSize;
    profile_bytes[profile_offset + 1U] = 3U;
    profile_bytes[profile_offset + 2U] = 5U;
    profile_bytes[profile_offset + 3U] = 3U;
    profile_bytes[profile_offset + 0x1EU] = 8U;
    profile_bytes[profile_offset + 0x1FU] = 4U;
    profile_bytes[profile_offset + 0x20U] = 7U;
    profile_bytes[profile_offset + 0x21U] = 3U;
    EntityActionProfileTable profiles{};
    if (!expect(copy_action_selection_profile_table(profile_bytes, profiles))) {
        return 101;
    }
    const auto* selected_profile =
        select_entity_action_profile_row(profiles, 2U);
    if (!expect(selected_profile != nullptr &&
                selected_profile->event_count_last_index == 3U &&
                selected_profile->events[0].code == 5U &&
                selected_profile->events[0].value == 3U &&
                selected_profile->primary_modifier().roll_count == 8U &&
                selected_profile->primary_modifier().difficulty == 4U &&
                selected_profile->secondary_modifier().roll_count == 7U &&
                selected_profile->secondary_modifier().difficulty == 3U &&
                select_entity_action_profile_row(profiles, 5U) == nullptr)) {
        return 102;
    }

    EntityActionRecord entity{};
    entity.item18_base = 7U;
    entity.item_codes[1] = 0x18U;
    entity.item_phases[1] = 3U;
    entity.item_codes[6] = 0x18U;
    entity.item_phases[6] = 2U;
    if (!expect(calculate_item18_skill_value(entity) == 9U)) {
        return 1;
    }
    entity.item_codes[6] = 0U;
    if (!expect(calculate_item18_skill_value(entity) == 10U)) {
        return 2;
    }

    // Slot 7 is not part of either ROM phase scan; it is reserved for the
    // separate special-item branch.
    EntityActionRecord slot7_only{};
    slot7_only.item18_base = 7U;
    slot7_only.item_codes[7] = 0x18U;
    slot7_only.item_phases[7] = 0x40U;
    if (!expect(calculate_item18_skill_value(slot7_only) == 7U &&
                add_first_matching_item_phase(5U, slot7_only, 0x18U) ==
                    5U)) {
        return 9;
    }

    if (!expect(select_posture_adjusted_value(20U, 2U, true) == 10U &&
                select_posture_adjusted_value(20U, 2U, false) == 10U &&
                select_posture_adjusted_value(9U, 1U, true) == 2U &&
                select_posture_adjusted_value(9U, 1U, false) == 7U)) {
        return 3;
    }

    entity.item_codes[6] = 0x1AU;
    entity.item_phases[6] = 4U;
    entity.item_codes[2] = 0x1AU;
    entity.item_phases[2] = 9U;
    if (!expect(add_first_matching_item_phase(5U, entity, 0x1AU) == 9U &&
                add_first_matching_item_phase(5U, entity, 0x1BU) == 5U &&
                add_first_matching_item_phase(0x1200U, entity, 0x1AU) ==
                    0x1204U)) {
        return 4;
    }

    EntityActionSpecialItemModifierTable item_modifiers{};
    item_modifiers.action_offsets[5] = 1;
    item_modifiers.slot7_item_modifiers[0][1] = -2;
    entity.item_codes[7] = 0x29U;
    const auto extended_phase = apply_action_item_phase_modifiers(
        5U, entity, 0x1AU, 5U, &item_modifiers);
    if (!expect(extended_phase.value == 7U && extended_phase.phase_found &&
                extended_phase.special_item_applied)) {
        return 5;
    }

    EntityActionRecord slot7_special{};
    slot7_special.item_codes[7] = 0x29U;
    slot7_special.item_phases[7] = 0x64U;
    const auto slot7_modifier_only = apply_action_item_phase_modifiers(
        5U, slot7_special, 0x29U, 5U, &item_modifiers);
    if (!expect(slot7_modifier_only.value == 3U &&
                !slot7_modifier_only.phase_found &&
                slot7_modifier_only.special_item_applied)) {
        return 10;
    }
    const auto extended_high_byte = apply_action_item_phase_modifiers(
        0x1200U, entity, 0x1AU, 5U, &item_modifiers);
    if (!expect(extended_high_byte.value == 0x1202U &&
                extended_high_byte.phase_found &&
                extended_high_byte.special_item_applied)) {
        return 14;
    }
    item_modifiers.action_offsets[5] = -1;
    const auto skipped_special = apply_action_item_phase_modifiers(
        5U, entity, 0x1AU, 5U, &item_modifiers);
    if (!expect(skipped_special.value == 9U &&
                !skipped_special.special_item_applied)) {
        return 6;
    }

    if (!expect(add_wound_difficulty_penalty(4U, 0U, 4U) == 8U &&
                add_wound_difficulty_penalty(4U, 5U, 7U) == 6U &&
                add_wound_difficulty_penalty(4U, 8U, 10U) == 4U)) {
        return 7;
    }

    EntityActionTarget target{};
    entity.cyberware_flags = 0x0002U;
    target.class_code = 0x16U;
    if (!expect(apply_target_condition_modifier(4U, entity, target) == 5U)) {
        return 8;
    }
    entity.cyberware_flags = 0U;
    entity.race_or_state = 0U;
    target.class_code = 0U;
    target.flags = 0x80U;
    if (!expect(apply_target_condition_modifier(4U, entity, target) == 8U)) {
        return 9;
    }
    target.flags = 0U;
    if (!expect(apply_target_condition_modifier(4U, entity, target) == 4U)) {
        return 10;
    }

    const EntityActionCatalogRecord action{1U, 10};
    const auto residual = calculate_action_residual(6U, 5U, 8U, action);
    if (!expect(residual.valid && residual.effective_difficulty == 4U &&
                residual.adjusted_roll_count == 2U &&
                residual.residual_effect == 8)) {
        return 11;
    }
    const auto hard_residual = calculate_action_residual(6U, 14U, 8U,
                                                          {0U, 10});
    if (!expect(hard_residual.effective_difficulty == 7U &&
                hard_residual.adjusted_roll_count == 0U &&
                hard_residual.residual_effect == 10)) {
        return 12;
    }
    const auto first_action_row = calculate_action_residual(0U, 4U, 8U,
                                                            action);
    if (!expect(first_action_row.valid &&
                first_action_row.effective_difficulty == 3U &&
                first_action_row.adjusted_roll_count == 3U &&
                first_action_row.residual_effect == 7)) {
        return 13;
    }

    EntityActionRecord ready{};
    ready.flags_10 = 0x01U;
    ready.class_code = 1U;
    ready.item18_base = 9U;
    ready.item_codes[6] = 0x18U;
    ready.item_phases[6] = 2U;
    EntityActionDynamicModifierContext dynamic{};
    dynamic.actor_flags_df = 0x01U;
    dynamic.actor_record_type = 2U;
    dynamic.actor_attribute_77 = 3U;
    dynamic.actor_attribute_7c = 8U;
    dynamic.ready_entities[0] = ready;
    dynamic.action_profile_table = &profiles;
    dynamic.linked_profile_index = 2U;
    dynamic.attribute_selectors[0] = -1;
    std::uint16_t dynamic_difficulty = 4U;
    const auto dynamic_result = apply_dynamic_status_modifier(
        dynamic_difficulty, dynamic,
        [](const std::uint16_t roll_count, const std::uint16_t difficulty) {
            return static_cast<std::uint8_t>(roll_count == 6U &&
                                             difficulty == 4U ? 2U : 0U);
        });
    if (!expect(dynamic_result.selected &&
                dynamic_result.action_profile_available &&
                dynamic_result.selected_profile_index == 2U &&
                dynamic_result.selected_action_table_index == 5U &&
                dynamic_result.ready_roll_count == 6U &&
                dynamic_result.added_difficulty == 6U &&
                dynamic_difficulty == 10U)) {
        return 14;
    }

    dynamic.actor_flags_df = 0x02U;
    dynamic.actor_record_type = 3U;
    dynamic.actor_attribute_7c = 2U;
    dynamic.attribute_selectors[1] = 1;
    dynamic_difficulty = 4U;
    const auto defensive_result = apply_dynamic_status_modifier(
        dynamic_difficulty, dynamic,
        [](const std::uint16_t roll_count, const std::uint16_t difficulty) {
            return static_cast<std::uint8_t>(roll_count == 2U &&
                                             difficulty == 3U ? 1U : 0U);
        });
    if (!expect(defensive_result.selected &&
                defensive_result.selected_action_table_index == 12U &&
                defensive_result.ready_roll_count == 2U &&
                defensive_result.added_difficulty == 3U &&
                dynamic_difficulty == 7U)) {
        return 15;
    }

    dynamic.action_profile_table = nullptr;
    dynamic_difficulty = 4U;
    const auto unresolved_profile = apply_dynamic_status_modifier(
        dynamic_difficulty, dynamic,
        [](const std::uint16_t, const std::uint16_t) {
            return static_cast<std::uint8_t>(99U);
        });
    if (!expect(unresolved_profile.selected &&
                !unresolved_profile.action_profile_available &&
                !unresolved_profile.roll_service_available &&
                dynamic_difficulty == 4U)) {
        return 160;
    }

    dynamic.actor_flags_df = 0U;
    dynamic.action_profile_table = &profiles;
    dynamic_difficulty = 4U;
    if (!expect(!apply_dynamic_status_modifier(dynamic_difficulty, dynamic,
                                               {}).selected &&
                dynamic_difficulty == 4U)) {
        return 16;
    }

    EntityActionRecord consumable{};
    consumable.item_codes[6] = 0x13U;
    consumable.item_phases[6] = 2U;
    consumable.item_codes[2] = 0x13U;
    consumable.item_phases[2] = 5U;
    const auto partial = consume_item13_residual(consumable, 3U);
    if (!expect(partial.result ==
                    EntityActionPhaseConsumptionResult::consumed_partially &&
                partial.remaining == 0U && consumable.item_phases[6] == 0U &&
                consumable.item_codes[6] == 0x00U &&
                consumable.item_phases[2] == 4U)) {
        return 17;
    }
    const auto exhausted = consume_item13_residual(consumable, 7U);
    if (!expect(exhausted.result ==
                    EntityActionPhaseConsumptionResult::inventory_exhausted &&
                exhausted.remaining == 3U && consumable.item_codes[2] == 0U &&
                consumable.item_phases[2] == 0U)) {
        return 18;
    }

    EntityActionActorState actor{};
    actor.record.item18_base = 8U;
    actor.record.posture = 1U;
    actor.record.attribute_7c = 4U;
    actor.record.physical_meter = 8U;
    actor.record.mental_meter = 8U;
    actor.roll_count_cap = 7U;
    actor.auxiliary_roll_bonus = 1U;
    actor.equipped_action_code = 6U;
    actor.record.item_codes[6] = 0x1EU;
    actor.record.item_phases[6] = 4U;
    actor.record.item_codes[5] = 0x13U;
    actor.record.item_phases[5] = 5U;
    actor.first_item_code = 0x22U;
    actor.record_type = 2U;
    EntityActionTarget action_target{};
    action_target.flags = 0U;
    const EntityActionCatalogRecord action_row{0U, 10};
    std::uint8_t spawn_count = 0U;
    std::uint8_t published_successes = 0U;
    std::uint8_t published_auxiliary = 0U;
    std::vector<std::int16_t> behavior_deltas;
    const EntityTargetActionServices action_services{
        [](const std::uint16_t roll_count, const std::uint16_t difficulty) {
            return static_cast<std::uint8_t>(roll_count == 7U &&
                                             difficulty == 4U ? 1U : 0U);
        },
        [&spawn_count](EntityActionActorState&) { ++spawn_count; },
        [&published_successes, &published_auxiliary](
            const std::uint8_t successes, const std::uint8_t auxiliary) {
            published_successes = successes;
            published_auxiliary = auxiliary;
        },
        [&behavior_deltas](EntityActionActorState&, const std::int16_t delta) {
            behavior_deltas.push_back(delta);
        }};
    const auto action_resolution = resolve_entity_target_action(
        actor, action_target, action_row, nullptr, action_services);
    if (!expect(action_resolution.outcome ==
                    EntityTargetActionOutcome::residual_consumed_exactly &&
                action_resolution.initial_roll_count == 7U &&
                action_resolution.difficulty == 4U &&
                action_resolution.successes == 1U && spawn_count == 1U &&
                published_successes == 1U && published_auxiliary == 1U &&
                actor.action_result == 1U && actor.record.item_codes[5] == 0U)) {
        return 19;
    }

    EntityActionActorState gated_actor{};
    gated_actor.record.mental_meter = 1U;
    gated_actor.record_type = 3U;
    gated_actor.equipped_slot = 0x7F;
    gated_actor.equipped_action_code = 0U;
    gated_actor.first_item_code = 0x1AU;
    const auto gated = resolve_entity_target_action(
        gated_actor, action_target, action_row, nullptr, {});
    if (!expect(gated.outcome == EntityTargetActionOutcome::target_state_gate &&
                gated_actor.action_result == 0U &&
                gated_actor.equipped_slot == 0 &&
                gated_actor.equipped_action_code == 0x1AU)) {
        return 20;
    }

    return 0;
}
