#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct EntityCoordinateTarget {
    std::uint16_t current = 0;
    std::uint16_t target = 0;
};

// ROM 0xB530: move a fixed-point coordinate toward its target in 0x800-unit
// steps, snapping to the target when the remaining distance is one step or
// less.
void approach_entity_fixed_point_target(EntityCoordinateTarget& state);

} // namespace shadowrun::ghidra_conversion
