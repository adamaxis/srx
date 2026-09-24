#include "shadowrun_ghidra/movement_route_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

std::uint16_t route_key_for_heading(const std::uint16_t heading) {
    auto rotated = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(heading << 3U) |
        static_cast<std::uint16_t>(heading >> 13U));
    rotated = static_cast<std::uint16_t>(rotated & 0x07U);
    return static_cast<std::uint16_t>((rotated & 0x06U) << 1U);
}

bool accepted_by_probe(
    const std::int16_t collision_code,
    const MovementResultPolicy& result_policy) {
    return apply_movement_result_policy(collision_code, result_policy) > 0;
}

} // namespace

bool evaluate_entity_movement_route(
    const std::uint16_t heading,
    const MovementResultPolicy& result_policy,
    const MovementRouteLookup& route_lookup,
    const MovementRouteProbe& probe) {
    const auto route = route_lookup(route_key_for_heading(heading));
    if (route.primary_marker > 0 && route.secondary_marker > 0) {
        return true;
    }

    const auto direction_slot =
        route.primary_marker <= 0 ? route.primary_fallback_slot
                                  : route.secondary_fallback_slot;
    return accepted_by_probe(probe(direction_slot), result_policy);
}

} // namespace shadowrun::ghidra_conversion
