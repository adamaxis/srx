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

struct TerrainMarkerReplacementState {
    std::uint16_t field_mode = 0;                 // FFDF40
    std::uint8_t effect_context_active = 0;       // FFE184
    std::uint8_t alternate_effect_context = 0;    // FFE187
    std::uint8_t effect_handler_index = 0;        // FFFBED
    std::uint16_t selected_effect_index = 0;      // FFE12C
    std::uint16_t active_effect_index = 0;        // FFF0C8
    std::uint8_t marker_ordinal = 0;              // FFE185
    bool world_effect_active = false;             // FFE180
};

// ROM 0xEE10: clear the 64x64 terrain-code map, rasterize each inclusive
// rectangle in 32-unit steps, and invoke the explicit post-processing
// boundary. The source rectangle table is represented as a bounded vector.
void rebuild_terrain_collision_map(
    std::array<std::uint8_t, 0x1000>& map,
    bool& rebuild_requested,
    const std::vector<TerrainCollisionRectangle>& rectangles,
    const TerrainMapPostProcess& post_process);

// ROM 0xEE86: when the active field/effect state selects the special terrain
// case, replace the requested occurrence of marker 0xDA with 0x9A. The host
// returns false for a malformed map that lacks the requested marker; the ROM
// would continue scanning because its source map is assumed to contain it.
bool replace_nth_terrain_marker(
    std::array<std::uint8_t, 0x1000>& map,
    const TerrainMarkerReplacementState& state);

// Low-level ordinal operation retained for extraction tests and callers that
// already performed the source gate outside this routine.
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
