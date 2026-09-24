#pragma once

#include "shadowrun_ghidra/movement_probe_conversion.hpp"

#include <cstdint>
#include <functional>

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
    std::function<std::int16_t(std::uint16_t direction_slot)>;

// ROM 0xBB86: derive the even heading-table key, accept a route when both
// source markers are active, or probe the appropriate fallback direction.
// Route data and collision probing remain native service boundaries.
bool evaluate_entity_movement_route(
    std::uint16_t heading,
    const MovementResultPolicy& result_policy,
    const MovementRouteLookup& route_lookup,
    const MovementRouteProbe& probe);

} // namespace shadowrun::ghidra_conversion
