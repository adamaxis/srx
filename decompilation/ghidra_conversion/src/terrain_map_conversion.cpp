#include "shadowrun_ghidra/terrain_map_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

void replace_nth_terrain_marker(
    std::array<std::uint8_t, 0x1000>& map,
    const std::uint8_t marker,
    const std::uint8_t replacement,
    std::uint16_t ordinal) {
    for (auto& value : map) {
        if (value != marker) {
            continue;
        }
        if (ordinal == 0U) {
            value = replacement;
            return;
        }
        --ordinal;
    }
}

void rebuild_terrain_collision_map(
    std::array<std::uint8_t, 0x1000>& map,
    const bool rebuild_requested,
    const std::vector<TerrainCollisionRectangle>& rectangles,
    const TerrainMapPostProcess& post_process) {
    if (!rebuild_requested) {
        return;
    }

    map.fill(0);
    for (const auto& rectangle : rectangles) {
        for (std::uint32_t y = rectangle.top;
             y <= rectangle.bottom; y += 0x20U) {
            for (std::uint32_t x = rectangle.left;
                 x <= rectangle.right; x += 0x20U) {
                const auto map_index = static_cast<std::size_t>(
                    ((((y & 0xFFE0U) << 1U) + (x >> 5U)) & 0x0FFFU));
                map[map_index] = rectangle.value;
            }
        }
    }
    post_process(map);
}

} // namespace shadowrun::ghidra_conversion
