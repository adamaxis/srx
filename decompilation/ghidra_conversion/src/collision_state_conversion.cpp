#include "shadowrun_ghidra/collision_state_conversion.hpp"

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
        std::nullopt, false, context.probe_x, context.probe_y};
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
                return result;
            }
            continue;
        }

        if (context.source_collision_override) {
            if ((context.source_flags & 0x08U) == 0U) {
                return result;
            }
            if (candidate_is_compatible(candidate)) {
                return result;
            }
            continue;
        }

        if ((context.source_flags & 0x08U) == 0U ||
            candidate_is_compatible(candidate)) {
            result.candidate_index = index;
            result.source_position_updated = true;
            result.source_x = candidate.world_x;
            result.source_y = static_cast<std::uint16_t>(
                candidate.world_y + 0x10U);
            return result;
        }
    }
    return result;
}

} // namespace shadowrun::ghidra_conversion
