#include "shadowrun_ghidra/entities/movement_state_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void reset_entity_movement_state(
    EntityMovementState& entity,
    const std::array<std::uint16_t, 8>& direction_animations,
    const std::uint16_t entity_x,
    const std::uint16_t entity_y,
    const CollisionEvaluator& collision_evaluator) {
    entity.movement_flags = static_cast<std::uint8_t>(
        entity.movement_flags & static_cast<std::uint8_t>(~0x02U));
    entity.current_heading = entity.desired_heading;
    entity.continuation = MovementContinuation::ReevaluateCollision;
    entity.animation_flags = static_cast<std::uint8_t>(
        entity.animation_flags | 0x01U);
    entity.movement_status = 0;

    auto direction = static_cast<std::uint16_t>(entity.current_heading << 3U |
                                                entity.current_heading >> 13U);
    if ((direction & 0x8000U) != 0U) {
        ++direction;
    }
    direction = static_cast<std::uint16_t>(direction & 0x07U);
    entity.direction_animation = direction_animations[direction];

    evaluate_entity_collision(entity.collision, entity_x, entity_y,
                              collision_evaluator);
}

} // namespace shadowrun::ghidra_conversion
