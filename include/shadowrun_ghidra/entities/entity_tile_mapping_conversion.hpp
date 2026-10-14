#pragma once

#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct EntityTilePair {
    std::uint16_t primary_coordinate = 0;
    std::uint16_t major_index = 0;
    std::uint16_t secondary_coordinate = 0;
};

// ROM 0x14640: split a halved world delta by the halved map stride and map
// the quotient/remainder into 0x200-spaced tile coordinates.
std::optional<EntityTilePair> map_world_delta_to_tile_pair(
    std::uint16_t world_delta,
    std::uint16_t map_stride);

} // namespace shadowrun::ghidra_conversion
