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

// ROM 0xEA36: test each sampled tile carrying the directional bit in source
// order. Each active corner is evaluated against the selected tile window;
// the first source carry ends the scan.
bool evaluate_entity_terrain_collision_directions(
    const TerrainTileSample& sample,
    std::uint16_t x,
    std::uint16_t y);

struct TerrainCollisionBounds {
    // These are the exclusive/inclusive-word boundaries compared with the
    // source coordinates after its four-bit normalization.
    std::uint16_t minimum_x_tile = 0;
    std::uint16_t exclusive_maximum_x_tile = 0;
    std::uint16_t minimum_y_tile = 0;
    std::uint16_t exclusive_maximum_y_tile = 0;
};

struct TerrainCollisionInput {
    std::uint16_t source_x = 0;
    std::uint16_t source_y = 0;
    std::uint8_t entity_flags = 0;
    TerrainCollisionBounds bounds{};
    // ROM FFEF10..13 persists the four tile bytes published by the previous
    // terrain sampler call. The center probe updates only byte zero; the
    // expanded probe updates all four bytes.
    std::array<std::uint8_t, 4> prior_sample_state{};
};

struct TerrainCollisionState {
    bool outside_bounds_marker = false;
    bool directional_collision_marker = false;
    std::int16_t raw_terrain_code = 0;
    TerrainTileSample sample{};
};

using TerrainCollisionSampleBoundary =
    std::function<TerrainTileSample(
        std::uint16_t x, std::uint16_t y, bool expanded_probe)>;
using TerrainCollisionObjectBoundary =
    std::function<bool(std::uint8_t object_id, std::int16_t x, std::int16_t y)>;
using TerrainCollisionDirectionBoundary =
    std::function<bool(const TerrainTileSample& sample,
                       std::uint16_t x,
                       std::uint16_t y)>;

struct TerrainCollisionCallbacks {
    TerrainCollisionSampleBoundary sample_tiles;
    TerrainCollisionObjectBoundary contains_object;
    TerrainCollisionDirectionBoundary evaluate_directions;
};

struct TerrainCollisionResult {
    std::int16_t code = 0;
    TerrainCollisionState state{};
    // The four bytes copied by ROM 0xB48C into the entity collision record.
    std::uint32_t packed_sample_state = 0;
};

// ROM 0xE7C8: normalize the source coordinate words, reject the four-sided
// terrain-map boundary, sample the center/four corners, then route the tile
// code through the object-bounds or directional collision leaf. The callbacks
// are host boundaries for decoded terrain records; no source RAM is exposed.
TerrainCollisionResult evaluate_terrain_collision(
    const TerrainCollisionInput& input,
    const TerrainCollisionCallbacks& callbacks);

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
