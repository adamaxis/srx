#pragma once

#include "shadowrun_ghidra/entities/movement_probe_conversion.hpp"

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct MovementFallbackState {
    std::uint16_t position_x = 0;
    std::uint16_t position_y = 0;
    std::uint16_t movement_status = 0; // +0x38
    std::uint16_t heading = 0;          // +0x92 interaction heading
    std::uint16_t target_heading = 0;   // +0x34
    std::uint32_t continuation = 0;     // +0xD4 restored into +0x3A
};

struct MovementFallbackAttempt {
    std::int16_t collision_code = 0;
    std::uint16_t candidate_x = 0;
    std::uint16_t candidate_y = 0;
};

using MovementFallbackProbe =
    std::function<MovementFallbackAttempt(std::uint16_t direction_slot)>;

// ROM 0xB49C: FFDF88 bit 0 selects +0x34 for the first probe only; later
// probes always add 0x4000/0x8000 to +0x92. A failed first or second probe
// (D0<=0 after 0xB732) commits D5/D6; the third probe never commits. +0xD4
// is restored into +0x3A on every exit.
void resolve_entity_fallback_movement(
    MovementFallbackState& entity,
    bool use_target_heading,
    const MovementResultPolicy& result_policy,
    const MovementFallbackProbe& probe,
    std::uint32_t restored_continuation);

} // namespace shadowrun::ghidra_conversion
