#pragma once

#include "shadowrun_ghidra/entities/movement_probe_conversion.hpp"

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct DirectionalMovementRoute {
    std::int8_t primary_marker = 0;
    std::int8_t secondary_marker = 0;
    std::uint16_t primary_fallback_slot = 0;
    std::uint16_t secondary_fallback_slot = 0;
};

using MovementRouteLookup =
    std::function<DirectionalMovementRoute(std::uint16_t route_key)>;
using MovementRouteProbe =
    std::function<MovementProbeResult(std::uint16_t direction_slot)>;

struct MovementRouteResult {
    // Preserve the ROM return flag because the caller commits D5/D6 only when
    // carry is clear. A fallback probe also leaves its candidate in D5/D6.
    bool carry_set = false;
    std::optional<MovementProbeResult> fallback_probe;
};

// ROM 0xBB86: derive the even heading-table key, preserve the source route
// decision, and retain a fallback collision candidate when one is probed.
// Route data and collision probing remain native service boundaries.
MovementRouteResult evaluate_entity_movement_route(
    std::uint16_t heading,
    const MovementResultPolicy& result_policy,
    const MovementRouteLookup& route_lookup,
    const MovementRouteProbe& probe);

} // namespace shadowrun::ghidra_conversion
