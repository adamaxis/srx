#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct TerrainCollisionRectangle {
    std::uint16_t left = 0;
    std::uint16_t right = 0;
    std::uint16_t top = 0;
    std::uint16_t bottom = 0;
    std::uint8_t value = 0;
};

using TerrainMapPostProcess =
    std::function<void(std::array<std::uint8_t, 0x1000>&)>;

// ROM 0xEE10: clear the 64x64 terrain-code map, rasterize each inclusive
// rectangle in 32-unit steps, and invoke the explicit post-processing
// boundary. The source rectangle table is represented as a bounded vector.
void rebuild_terrain_collision_map(
    std::array<std::uint8_t, 0x1000>& map,
    bool rebuild_requested,
    const std::vector<TerrainCollisionRectangle>& rectangles,
    const TerrainMapPostProcess& post_process);

// ROM 0xEE86: replace the requested zero-based occurrence of a terrain
// marker, preserving the source's first-marker behavior for ordinal zero.
void replace_nth_terrain_marker(
    std::array<std::uint8_t, 0x1000>& map,
    std::uint8_t marker,
    std::uint8_t replacement,
    std::uint16_t ordinal);

// ROM 0x16EA0: fill a source rectangular work-map region with terrain value
// 2. Host indices are expressed in cells and are bounded by the container.
void fill_terrain_work_region(
    std::vector<std::uint16_t>& map,
    std::size_t row_stride,
    std::size_t start_index,
    std::size_t width,
    std::size_t height);

// ROM 0x16FA8: copy a rectangular region from the decoded terrain source
// into the work map. Both buffers use the source row stride; bounds checks
// make the host operation safe for partial decoded resources.
void copy_terrain_work_region(
    std::vector<std::uint16_t>& destination,
    const std::vector<std::uint16_t>& source,
    std::size_t row_stride,
    std::size_t source_index,
    std::size_t destination_index,
    std::size_t width,
    std::size_t height);

} // namespace shadowrun::ghidra_conversion
