#pragma once

#include "shadowrun_ghidra/entities/movement_probe_conversion.hpp"

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct EntityMovementStepState {
    std::uint16_t position_x = 0;
    std::uint16_t position_y = 0;
    std::uint16_t heading = 0;
    std::uint16_t collision_context = 0;
    std::uint16_t movement_substate = 0;
    std::int16_t route_status = 0;
    std::uint16_t animation_heading = 0;
    std::uint8_t movement_flags = 0;
    std::uint8_t collision_flags = 0;
    std::uint8_t event_flags = 0;
};

using MovementStepProbe =
    std::function<MovementProbeResult(std::uint16_t direction_slot)>;
using MovementPolicyProbe =
    std::function<std::int16_t(std::uint16_t direction_slot)>;
using NegativeCollisionRecovery = std::function<bool()>;
using AlignedRouteCheck = std::function<bool()>;
using MovementReset = std::function<void()>;

// ROM 0xBADE: transfer linked collision context, probe the requested movement
// direction, handle recoverable negative collisions, choose aligned/diagonal
// movement fallbacks, and either commit the candidate position or reset
// movement. Collision tables and recovery services remain injected.
void advance_entity_movement_step(
    EntityMovementStepState& entity,
    std::uint16_t linked_collision_context,
    std::uint16_t direction_slot,
    const MovementResultPolicy& result_policy,
    const MovementStepProbe& probe,
    const NegativeCollisionRecovery& recover_negative_collision,
    const AlignedRouteCheck& check_aligned_route,
    const MovementPolicyProbe& policy_probe,
    const MovementReset& reset_movement);

} // namespace shadowrun::ghidra_conversion
