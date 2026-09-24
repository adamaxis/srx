#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct TerrainWorkEntry {
    std::uint16_t width = 0;
    std::uint16_t height = 0;
    std::int16_t region_offset = 0;
    std::size_t descriptor_span = 1;
};

struct TerrainWorkDispatchState {
    std::size_t entry_index = 0;
    std::size_t status_index = 0;
};

using TerrainWorkFillBoundary =
    std::function<void(const TerrainWorkEntry&, std::size_t status_index)>;

// ROM 0x16E5C: walk the variable-span terrain work descriptors until the
// zero-width terminator, filling only descriptors whose parallel status byte
// is clear. The host descriptor span makes the source's 3/6-word stepping
// explicit without exposing ROM pointers.
std::uint16_t process_terrain_work_entries(
    TerrainWorkDispatchState& state,
    const std::vector<TerrainWorkEntry>& entries,
    const std::vector<std::uint8_t>& status_bytes,
    const TerrainWorkFillBoundary& fill_entry);

struct TerrainWorkRegionRequest {
    std::size_t width = 0;
    std::size_t height = 0;
    std::size_t row_stride = 0;
    std::size_t source_index = 0;
    std::size_t destination_index = 0;
    std::int16_t mode = 0;
};

using TerrainWorkRegionResolveBoundary =
    std::function<TerrainWorkRegionRequest(const TerrainWorkRegionRequest&)>;

// ROM 0x16F8E: optionally resolve a negative-mode region, then copy its
// bounded rows from the decoded source into the terrain work map.
void copy_terrain_work_region_rows(
    std::vector<std::uint16_t>& destination,
    const std::vector<std::uint16_t>& source,
    TerrainWorkRegionRequest request,
    const TerrainWorkRegionResolveBoundary& resolve_region);

struct TerrainBoundary {
    std::uint16_t left = 0;
    std::uint16_t top = 0;
    std::uint16_t width_tiles = 0;
    std::uint16_t height_tiles = 0;
};

// ROM 0x16F3C: map the world delta and mark the destination byte when the
// resulting tile rectangle contains the requested terrain boundary.
bool mark_terrain_boundary_if_inside(
    std::uint16_t world_delta,
    std::uint16_t map_stride,
    const TerrainBoundary& boundary,
    std::uint8_t& destination_marker);

// ROM 0x16F14: use the regular check once, or repeat it through the explicit
// alternate boundary for the source's negative-mode path.
bool mark_terrain_boundary_for_mode(
    bool negative_mode,
    const std::function<bool()>& regular_check,
    const std::function<bool()>& alternate_check);

} // namespace shadowrun::ghidra_conversion
