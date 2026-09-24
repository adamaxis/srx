#pragma once

#include <array>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

// ROM 0x14902: map world coordinates into the source obstruction-grid index.
// The source grid uses 0x200 horizontal units and a 0x40-byte row stride.
std::uint16_t compute_obstruction_grid_index(
    std::uint16_t world_x,
    std::uint16_t world_y);

// ROM 0x14C2E: set bit 0 when X is no greater than its target and bit 1 when
// Y is no greater than its target.
std::uint8_t derive_entity_axis_relation_flags(
    std::uint16_t entity_x,
    std::uint16_t entity_y,
    std::uint16_t target_x,
    std::uint16_t target_y);

// ROM 0x15A8C: return the first available source slot in the six-slot range
// 9..14, or the one-past-last value 15 when every slot is occupied.
std::uint8_t find_first_available_entity_slot(
    const std::array<std::uint8_t, 6>& slot_flags);

} // namespace shadowrun::ghidra_conversion
