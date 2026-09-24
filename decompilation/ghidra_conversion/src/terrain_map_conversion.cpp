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

void fill_terrain_work_region(
    std::vector<std::uint16_t>& map,
    const std::size_t row_stride,
    const std::size_t start_index,
    const std::size_t width,
    const std::size_t height) {
    for (std::size_t row = 0; row < height; ++row) {
        for (std::size_t column = 0; column < width; ++column) {
            const auto index = start_index + row * row_stride + column;
            if (index < map.size()) {
                map[index] = 2U;
            }
        }
    }
}

} // namespace shadowrun::ghidra_conversion
