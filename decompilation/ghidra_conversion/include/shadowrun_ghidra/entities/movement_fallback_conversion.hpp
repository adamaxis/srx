#pragma once

#include "shadowrun_ghidra/entities/movement_probe_conversion.hpp"

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct MovementFallbackState {
    std::uint16_t position_x = 0;
    std::uint16_t position_y = 0;
    std::uint16_t movement_status = 0;
    std::uint16_t heading = 0;
    std::uint16_t target_heading = 0;
    std::uint32_t continuation = 0;
};

struct MovementFallbackAttempt {
    std::int16_t collision_code = 0;
    std::uint16_t candidate_x = 0;
    std::uint16_t candidate_y = 0;
};

using MovementFallbackProbe =
    std::function<MovementFallbackAttempt(std::uint16_t direction_slot)>;

// ROM 0xB49C: try the source heading and its quarter/half-turn alternatives
// through the movement-result policy. A failed first or second probe commits
// that probe's candidate position; the saved continuation is restored on every
// exit. The probe callback owns native collision queries and candidate
// generation; this conversion does not recreate ROM tables.
void resolve_entity_fallback_movement(
    MovementFallbackState& entity,
    bool use_target_heading,
    const MovementResultPolicy& result_policy,
    const MovementFallbackProbe& probe,
    std::uint32_t restored_continuation);

} // namespace shadowrun::ghidra_conversion
