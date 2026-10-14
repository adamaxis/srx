#pragma once

#include "shadowrun_ghidra/entities/movement_probe_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_route_conversion.hpp"

#include <array>
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
    std::int16_t collision_extent = 0;
};

// ROM 0xC002, sixteen signed words (eight X/Y pairs). 0xBADE reads
// MOVE.W 4(A1,D0) with D0 already direction*2.
constexpr std::array<std::int16_t, 16> kFieldMovementDirectionTableWords{
    0, -40, 40, -40, 40, 0, 40, 40, 0, 40, -40, 40, -40, 0, -40, -40};

inline std::uint16_t field_movement_step_context_word(
    const std::uint16_t direction_slot) {
    const auto index = static_cast<std::size_t>(
        (4U + static_cast<std::uint16_t>(direction_slot & 0x0EU)) >> 1U);
    return static_cast<std::uint16_t>(kFieldMovementDirectionTableWords[index]);
}

using MovementStepProbe =
    std::function<MovementProbeResult(std::uint16_t direction_slot)>;
using MovementPolicyProbe = MovementStepProbe;
using NegativeCollisionRecovery = std::function<bool()>;
using AlignedRouteCheck = std::function<MovementRouteResult()>;
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

// ROM 0xBABC: write +8=-12, rotate +0x2E into the 0xC002 slot, store
// 4(A1,D0) at +0xCA, then fall into 0xBADE.
void begin_entity_movement_pass(
    EntityMovementStepState& entity,
    const MovementResultPolicy& result_policy,
    const MovementStepProbe& probe,
    const NegativeCollisionRecovery& recover_negative_collision,
    const AlignedRouteCheck& check_aligned_route,
    const MovementPolicyProbe& policy_probe,
    const MovementReset& reset_movement);

} // namespace shadowrun::ghidra_conversion
