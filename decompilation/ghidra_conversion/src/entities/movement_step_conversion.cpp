#include "shadowrun_ghidra/entities/movement_step_conversion.hpp"

#include "shadowrun_ghidra/field/field_movement_input_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

bool policy_probe_is_blocked(
    const MovementProbeResult& raw_result,
    const MovementResultPolicy& result_policy) {
    return apply_movement_result_policy(
               raw_result.collision_code, result_policy) > 0;
}

void commit_candidate(
    EntityMovementStepState& entity,
    const MovementProbeResult& candidate) {
    entity.position_x = candidate.candidate_x;
    entity.position_y = candidate.candidate_y;
    entity.event_flags = static_cast<std::uint8_t>(
        entity.event_flags & static_cast<std::uint8_t>(~0x01U));
}

std::uint16_t rotate_right_four(const std::uint16_t value) {
    return static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(value >> 4U) |
        static_cast<std::uint16_t>(value << 12U));
}

} // namespace

void advance_entity_movement_step(
    EntityMovementStepState& entity,
    const std::uint16_t linked_collision_context,
    const std::uint16_t direction_slot,
    const MovementResultPolicy& result_policy,
    const MovementStepProbe& probe,
    const NegativeCollisionRecovery& recover_negative_collision,
    const AlignedRouteCheck& check_aligned_route,
    const MovementPolicyProbe& policy_probe,
    const MovementReset& reset_movement) {
    entity.collision_context = linked_collision_context;
    const auto initial = probe(direction_slot);

    if (initial.collision_code == 0 ||
        (initial.collision_code < 0 && !recover_negative_collision())) {
        entity.movement_substate = 0x1000U;
        commit_candidate(entity, initial);
        return;
    }

    if ((entity.heading & 0x3FFFU) == 0U) {
        const auto route_result = check_aligned_route();
        if (!route_result.carry_set &&
            route_result.fallback_probe.has_value()) {
            entity.collision_flags = static_cast<std::uint8_t>(
                entity.collision_flags & static_cast<std::uint8_t>(~0x01U));
            commit_candidate(entity, *route_result.fallback_probe);
            return;
        }

        entity.route_status = -1;
        reset_movement();
        return;
    }

    const auto movement_direction = static_cast<std::uint16_t>(
        (entity.heading >> 13U) & 0x06U);
    entity.movement_substate = movement_direction;
    auto selected_probe = policy_probe(
        static_cast<std::uint16_t>(movement_direction << 1U));
    if (policy_probe_is_blocked(selected_probe, result_policy)) {
        entity.route_status = static_cast<std::int16_t>(movement_direction);
        const auto alternate_direction = static_cast<std::uint16_t>(
            (movement_direction + 2U) & 0x07U);
        selected_probe = policy_probe(
            static_cast<std::uint16_t>(alternate_direction << 1U));
        if (policy_probe_is_blocked(selected_probe, result_policy)) {
            entity.route_status = -1;
            reset_movement();
            return;
        }
    }

    entity.animation_heading = rotate_right_four(
        selected_probe.source_direction_slot);
    entity.movement_flags = static_cast<std::uint8_t>(
        entity.movement_flags | 0x04U);
    commit_candidate(entity, selected_probe);
}

void begin_entity_movement_pass(
    EntityMovementStepState& entity,
    const MovementResultPolicy& result_policy,
    const MovementStepProbe& probe,
    const NegativeCollisionRecovery& recover_negative_collision,
    const AlignedRouteCheck& check_aligned_route,
    const MovementPolicyProbe& policy_probe,
    const MovementReset& reset_movement) {
    entity.collision_extent = -12;
    const auto direction = field_movement_direction_from_heading(entity.heading);
    const auto direction_slot =
        static_cast<std::uint16_t>(direction * 2U);
    advance_entity_movement_step(
        entity, field_movement_step_context_word(direction_slot),
        direction_slot, result_policy, probe, recover_negative_collision,
        check_aligned_route, policy_probe, reset_movement);
}

} // namespace shadowrun::ghidra_conversion
