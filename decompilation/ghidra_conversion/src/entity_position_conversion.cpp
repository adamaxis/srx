#include "shadowrun_ghidra/entity_position_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void commit_entity_position(EntityPositionState& state,
                            const std::uint16_t x,
                            const std::uint16_t y) {
    state.transition_flags = static_cast<std::uint8_t>(
        state.transition_flags & 0xFEU);
    state.x = x;
    state.y = y;
}

void copy_entity_anchor_position(
    EntityAnchorPosition& destination,
    const EntityAnchorPosition& source) {
    destination = source;
}

} // namespace shadowrun::ghidra_conversion
