#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct EntityPositionState {
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint8_t transition_flags = 0;
};

// ROM 0xBBE4: commit the supplied position and clear the entity's pending
// transition bit.
void commit_entity_position(EntityPositionState& state,
                            std::uint16_t x,
                            std::uint16_t y);

} // namespace shadowrun::ghidra_conversion
