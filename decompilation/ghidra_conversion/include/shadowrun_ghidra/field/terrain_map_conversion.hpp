#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <vector>

#include "shadowrun_ghidra/field/terrain_workflow_conversion.hpp"

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

// ROM 0x0313A: copy a decoded terrain layer into its work buffer. The caller
// supplies the bounded host containers and source word count.
void copy_terrain_layer(
    std::vector<std::uint16_t>& destination,
    const std::vector<std::uint16_t>& source,
    std::size_t word_count);

} // namespace shadowrun::ghidra_conversion
