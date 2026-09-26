#include "shadowrun_ghidra/entities/collision_state_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

std::int16_t evaluate_entity_collision(
    EntityCollisionRecord& entity,
    const std::uint16_t candidate_x,
    const std::uint16_t candidate_y,
    const CollisionEvaluator& collision_evaluator) {
    const auto evaluation = collision_evaluator(candidate_x, candidate_y);
    entity.packed_state = evaluation.packed_state;
    return evaluation.result;
}

namespace {

bool within_half_open_word_window(const std::uint16_t value,
                                  const std::uint16_t center) {
    const auto lower = static_cast<std::uint16_t>(center - 0x90U);
    const auto upper = static_cast<std::uint16_t>(center + 0x90U);
    return lower <= value && value < upper;
}

bool candidate_is_compatible(const CollisionCandidate& candidate) {
    return (candidate.interaction_flags & 0x80U) == 0U &&
        (candidate.flags & 0x08U) == 0U &&
        (candidate.behavior_flags & 0x10U) == 0U;
}

} // namespace

EntityCollisionSearchResult find_colliding_entity(
    const EntityCollisionSearchContext& context,
    const std::vector<CollisionCandidate>& candidates) {
    EntityCollisionSearchResult result{
        std::nullopt, false, false, context.probe_x, context.probe_y};
    const auto scan_count = std::min<std::size_t>(candidates.size(), 10U);
    for (std::size_t index = 0; index < scan_count; ++index) {
        const auto& candidate = candidates[index];
        if (!candidate.collision_enabled ||
            (candidate.flags & 0x01U) == 0U ||
            (candidate.flags & 0x40U) != 0U ||
            (candidate.flags & 0x10U) != 0U ||
            !within_half_open_word_window(candidate.world_x, context.probe_x) ||
            !within_half_open_word_window(candidate.world_y, context.probe_y)) {
            continue;
        }

        if (context.collision_gate_active) {
            if (context.excluded_candidate.has_value() &&
                context.excluded_candidate.value() == index) {
                // 0x15B18-0x15B2E: the protected source record is a terminal
                // carry exit, not a selected collision candidate.
                result.source_carry = true;
                return result;
            }
            continue;
        }

        if (context.source_collision_override) {
            if ((context.source_flags & 0x08U) == 0U) {
                // 0x15B78-0x15B80: source mode exits with carry before the
                // candidate compatibility checks.
                result.source_carry = true;
                return result;
            }
            if (candidate_is_compatible(candidate)) {
                // 0x15B96-0x15BA0: the compatible candidate takes the
                // source's carry-only continuation; it does not overwrite
                // the source coordinates in this mode.
                result.source_carry = true;
                return result;
            }
            continue;
        }

        if ((context.source_flags & 0x08U) == 0U ||
            candidate_is_compatible(candidate)) {
            result.candidate_index = index;
            result.source_carry = true;
            result.source_position_updated = true;
            result.source_x = candidate.world_x;
            result.source_y = static_cast<std::uint16_t>(
                candidate.world_y + 0x10U);
            return result;
        }
    }
    return result;
}

namespace {

std::uint16_t fixed_point_integer_word(const std::int32_t value) {
    return static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(value) >> 16U);
}

} // namespace

EntityCollisionRecoveryResult advance_entity_collision_recovery(
    EntityCollisionRecoveryState& state,
    const bool terrain_boundary_active,
    const EntityWorldProbe& terrain_probe,
    EntityCollisionSearchContext collision_context,
    const std::vector<CollisionCandidate>& candidates) {
    const auto probe = probe_entity_world_position(
        state.position, terrain_probe);
    EntityCollisionRecoveryResult result{
        EntityCollisionRecoveryAction::terrain_probe_blocked,
        false,
        probe.terrain_result,
        probe.world_x,
        probe.world_y,
        std::nullopt};

    if (probe.terrain_result != 0U && !terrain_boundary_active) {
        result.source_carry = true;
        return result;
    }

    --state.retry_counter;
    if (state.retry_counter < 0) {
        result.action = EntityCollisionRecoveryAction::retry_exhausted;
        result.source_carry = true;
        return result;
    }

    state.position.world_x = probe.world_x;
    state.position.world_y = probe.world_y;
    collision_context.probe_x = fixed_point_integer_word(probe.world_x);
    collision_context.probe_y = fixed_point_integer_word(probe.world_y);
    result.action = EntityCollisionRecoveryAction::collision_search;
    result.collision_search = find_colliding_entity(
        collision_context, candidates);
    result.source_carry = result.collision_search->source_carry;
    return result;
}

} // namespace shadowrun::ghidra_conversion
