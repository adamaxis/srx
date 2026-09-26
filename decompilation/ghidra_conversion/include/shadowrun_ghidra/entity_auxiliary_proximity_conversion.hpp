#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct EntityAuxiliaryProximitySource {
    std::uint16_t world_x = 0;
    std::uint16_t world_y = 0;
    std::uint8_t behavior_type = 0; // source record +0x57
};

struct EntityAuxiliaryProximityCandidate {
    bool active = false; // source candidate +0x10 bit 0
    bool excluded = false; // source candidate +0x10 bit 6
    std::uint16_t world_x = 0; // source candidate +0x00
    std::uint16_t world_y = 0; // source candidate +0x04
};

struct EntityAuxiliaryProximityCallbacks {
    // ROM 0x1562E: service_entity_auxiliary_behavior(candidate).
    std::function<void(std::size_t)> service_candidate;
};

struct EntityAuxiliaryProximityResult {
    std::size_t serviced_candidates = 0;
};

// ROM 0x15554: find active, non-excluded entities in the source's local
// behavior window and hand each one to the existing auxiliary behavior
// service. Type 0x0E uses the half-size window; all other types use the full
// window. The source clamps the lower edge at world coordinate zero and uses
// an exclusive upper edge.
EntityAuxiliaryProximityResult service_nearby_entity_auxiliary_behavior(
    const EntityAuxiliaryProximitySource& source,
    const std::array<EntityAuxiliaryProximityCandidate, 10>& candidates,
    const EntityAuxiliaryProximityCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
