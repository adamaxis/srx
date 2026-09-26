#include "shadowrun_ghidra/field/terrain_workflow_conversion.hpp"

#include "shadowrun_ghidra/entities/entity_tile_mapping_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void fill_terrain_work_region(
    std::vector<std::uint16_t>& destination,
    const std::size_t width,
    const std::size_t height,
    const std::size_t row_stride,
    const std::size_t destination_index) {
    for (std::size_t row = 0; row < height; ++row) {
        for (std::size_t column = 0; column < width; ++column) {
            const auto index = destination_index + row * row_stride + column;
            if (index < destination.size()) {
                destination[index] = 2U;
            }
        }
    }
}

void copy_terrain_work_region(
    std::vector<std::uint16_t>& destination,
    const std::vector<std::uint16_t>& source,
    const std::size_t width,
    const std::size_t height,
    const std::size_t row_stride,
    const std::size_t destination_index,
    const std::size_t source_index) {
    for (std::size_t row = 0; row < height; ++row) {
        for (std::size_t column = 0; column < width; ++column) {
            const auto source_offset = source_index + row * row_stride + column;
            const auto destination_offset =
                destination_index + row * row_stride + column;
            if (source_offset < source.size() &&
                destination_offset < destination.size()) {
                destination[destination_offset] = source[source_offset];
            }
        }
    }
}

std::uint16_t process_terrain_work_entries(
    TerrainWorkDispatchState& state,
    const std::vector<TerrainWorkEntry>& entries,
    const std::vector<std::uint8_t>& status_bytes,
    const TerrainWorkFillBoundary& fill_entry) {
    while (state.entry_index < entries.size()) {
        const auto& entry = entries[state.entry_index];
        if (entry.width == 0U) {
            return entry.width;
        }

        const auto status = state.status_index < status_bytes.size()
                                ? status_bytes[state.status_index]
                                : 0U;
        if (status == 0U && fill_entry) {
            fill_entry(entry, state.status_index);
        }

        state.entry_index += entry.descriptor_span == 0U
                                 ? 1U
                                 : entry.descriptor_span;
        if (entry.region_offset >= 0) {
            ++state.status_index;
        }
    }
    return 0U;
}

TerrainWorkBatchResult process_terrain_work_batch(
    TerrainWorkDispatchState& state,
    const std::vector<TerrainWorkEntry>& entries,
    const std::vector<std::uint8_t>& status_bytes,
    const TerrainBoundaryMarkBoundary& mark_boundary,
    const TerrainWorkCopyBoundary& copy_region) {
    TerrainWorkBatchResult result{};
    while (state.entry_index < entries.size()) {
        const auto& entry = entries[state.entry_index];
        if (entry.width == 0U) {
            result.terminated = true;
            return result;
        }

        const auto status = state.status_index < status_bytes.size()
                                ? status_bytes[state.status_index]
                                : 0U;
        const auto span = entry.descriptor_span == 0U
                              ? 1U
                              : entry.descriptor_span;
        if (status == 0U && mark_boundary &&
            mark_boundary(entry, state.status_index)) {
            if (copy_region) {
                copy_region(entry);
            }
            result.copied = true;
            result.map_dirty = true;
            return result;
        }

        state.entry_index += span;
        if (entry.region_offset >= 0) {
            ++state.status_index;
        }
    }
    result.terminated = true;
    return result;
}

void copy_terrain_work_region_rows(
    std::vector<std::uint16_t>& destination,
    const std::vector<std::uint16_t>& source,
    TerrainWorkRegionRequest request,
    const TerrainWorkRegionResolveBoundary& resolve_region) {
    if (request.mode < 0 && resolve_region) {
        request = resolve_region(request);
    }
    for (std::size_t row = 0; row < request.height; ++row) {
        for (std::size_t column = 0; column < request.width; ++column) {
            const auto source_index = request.source_index +
                                      row * request.row_stride + column;
            const auto destination_index = request.destination_index +
                                           row * request.row_stride + column;
            if (source_index < source.size() &&
                destination_index < destination.size()) {
                destination[destination_index] = source[source_index];
            }
        }
    }
}

bool mark_terrain_boundary_if_inside(
    const std::uint16_t world_delta,
    const std::uint16_t map_stride,
    const TerrainBoundary& boundary,
    std::uint8_t& destination_marker) {
    const auto mapped = map_world_delta_to_tile_pair(world_delta, map_stride);
    if (!mapped.has_value()) {
        return false;
    }

    const auto left = static_cast<std::uint32_t>(boundary.left);
    const auto top = static_cast<std::uint32_t>(boundary.top);
    const auto right = left +
                       ((static_cast<std::uint32_t>(boundary.width_tiles) +
                         1U) *
                        0x0200U);
    const auto bottom = top + 0x0200U +
                        (static_cast<std::uint32_t>(boundary.height_tiles) *
                         0x0200U);
    const auto primary = static_cast<std::uint32_t>(mapped->primary_coordinate);
    const auto secondary =
        static_cast<std::uint32_t>(mapped->secondary_coordinate);
    if (primary >= left && secondary >= top + 0x0200U && primary < right &&
        secondary < bottom) {
        destination_marker = 0xFFU;
        return true;
    }
    return false;
}

bool mark_terrain_boundary_for_mode(
    const bool negative_mode,
    const std::function<bool()>& regular_check,
    const std::function<bool()>& alternate_check) {
    if (!negative_mode) {
        return regular_check && regular_check();
    }
    if (regular_check && regular_check()) {
        return true;
    }
    return alternate_check && alternate_check();
}

} // namespace shadowrun::ghidra_conversion
