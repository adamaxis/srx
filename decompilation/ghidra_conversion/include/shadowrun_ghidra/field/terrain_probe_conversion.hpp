#pragma once

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

using TerrainTileLookup =
    std::function<std::uint8_t(std::uint16_t tile_index)>;

struct TerrainTileSample {
    std::array<std::uint8_t, 4> values{};
    std::uint8_t selected_value = 0;
    std::uint16_t selected_tile_index = 0;
    bool contains_blocked_tile = false;
};

// ROM 0xE8A0: sample the center tile or four offset tiles, retain the
// highest unsigned terrain code (later ties win), and let 0x7F override the
// selected value. The tile map is supplied as a host lookup.
TerrainTileSample sample_terrain_collision_tiles(
    std::uint16_t x,
    std::uint16_t y,
    bool expanded_probe,
    const TerrainTileLookup& tile_lookup);

struct TerrainObjectBounds {
    std::int16_t center_x = 0;
    std::int16_t center_y = 0;
};

using TerrainObjectLookup =
    std::function<TerrainObjectBounds(std::uint16_t object_id)>;

// ROM 0xE9EE: test whether a point is within the selected terrain object's
// center +/-12 horizontal and +/-7 vertical bounds.
bool contains_terrain_object_bounds(
    std::uint16_t object_id,
    std::int16_t x,
    std::int16_t y,
    const TerrainObjectLookup& object_lookup);

struct TerrainTileWindow {
    std::uint16_t origin_x = 0;
    std::uint16_t origin_y = 0;
    std::uint16_t end_y = 0;
};

// ROM 0xEDFA: expand the packed terrain tile index into its 32-unit
// horizontal origin, row origin, and row end.
TerrainTileWindow decode_terrain_tile_window(std::uint16_t tile_index);

// ROM 0xEAD2: compare a probe point against the decoded tile window using
// the directional relation encoded in terrain-code bits 0x38. All
// subtractions and comparisons retain the source unsigned-word behavior.
bool evaluate_terrain_direction_boundary(
    std::uint8_t terrain_code,
    const TerrainTileWindow& window,
    std::uint16_t x,
    std::uint16_t y);

} // namespace shadowrun::ghidra_conversion
