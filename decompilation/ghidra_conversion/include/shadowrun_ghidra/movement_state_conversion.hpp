#pragma once

#include "shadowrun_ghidra/collision_state_conversion.hpp"

#include <array>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

enum class MovementContinuation {
    ReevaluateCollision,
};

struct EntityMovementState {
    std::uint8_t movement_flags = 0;
    std::uint16_t current_heading = 0;
    std::uint16_t desired_heading = 0;
    MovementContinuation continuation =
        MovementContinuation::ReevaluateCollision;
    std::uint8_t animation_flags = 0;
    std::uint16_t movement_status = 0;
    std::uint16_t direction_animation = 0;
    EntityCollisionRecord collision{};
};

// ROM 0xB442: clear the blocked bit, restore the desired heading, schedule the
// native collision-recheck continuation, refresh the directional animation,
// and evaluate the entity's current position.
void reset_entity_movement_state(
    EntityMovementState& entity,
    const std::array<std::uint16_t, 8>& direction_animations,
    std::uint16_t entity_x,
    std::uint16_t entity_y,
    const CollisionEvaluator& collision_evaluator);

} // namespace shadowrun::ghidra_conversion
