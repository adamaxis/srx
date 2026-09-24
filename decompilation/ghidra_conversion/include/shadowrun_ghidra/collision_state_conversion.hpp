#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct CollisionEvaluation {
    std::int16_t result = 0;
    std::uint32_t packed_state = 0;
};

struct EntityCollisionRecord {
    std::uint32_t packed_state = 0;
};

using CollisionEvaluator = std::function<CollisionEvaluation(
    std::uint16_t candidate_x, std::uint16_t candidate_y)>;

// ROM 0xB48C: pass the candidate coordinates to the field-collision boundary,
// copy its four-byte collision state into the entity record, and return its
// collision code. The boundary remains an explicit native service interface;
// this conversion does not recreate the ROM collision table or hardware.
std::int16_t evaluate_entity_collision(
    EntityCollisionRecord& entity,
    std::uint16_t candidate_x,
    std::uint16_t candidate_y,
    const CollisionEvaluator& collision_evaluator);

} // namespace shadowrun::ghidra_conversion
