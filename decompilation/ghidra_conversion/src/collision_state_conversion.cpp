#include "shadowrun_ghidra/collision_state_conversion.hpp"

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

} // namespace shadowrun::ghidra_conversion
