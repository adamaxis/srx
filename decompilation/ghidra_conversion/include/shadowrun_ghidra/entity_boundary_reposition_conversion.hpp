#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct EntityBoundaryRepositionState {
    std::uint8_t flags = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
};

struct EntityBoundaryRepositionInput {
    std::uint16_t horizontal_origin = 0;
    std::uint16_t vertical_origin = 0;
    std::uint16_t horizontal_midpoint = 0;
    std::uint16_t vertical_midpoint = 0;
};

// ROM 0x13E9E: set the one-shot reposition flag and place the entity one
// 0x2580 step to either side of each source 0xA00-offset boundary.
bool reposition_entity_at_boundary(
    EntityBoundaryRepositionState& state,
    const EntityBoundaryRepositionInput& input);

} // namespace shadowrun::ghidra_conversion
