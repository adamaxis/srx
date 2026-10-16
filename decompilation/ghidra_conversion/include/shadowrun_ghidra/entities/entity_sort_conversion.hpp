#pragma once

#include <array>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct EntityDepthEntry {
    std::int16_t depth = 0;
    std::uint8_t flags = 0;
    std::uint16_t id = 0;
};

// ROM 0x5B1E: stably order the sixteen entity slots from greatest effective
// depth to least. Source flags 0x10 and 0x40 force an entity's sort depth to
// zero before comparison.
void sort_entity_slots_by_depth(
    std::array<EntityDepthEntry, 16>& entities);

} // namespace shadowrun::ghidra_conversion
