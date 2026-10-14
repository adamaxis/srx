#include "shadowrun_ghidra/field/terrain_workflow_conversion.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TerrainWorkDispatchState dispatch{};
    const std::vector<TerrainWorkEntry> entries{
        {2, 2, 0, 1}, {3, 1, -1, 2}, {0, 0, 0, 1}};
    const std::vector<std::uint8_t> statuses{0, 1};
    std::vector<std::size_t> filled;
    if (process_terrain_work_entries(
            dispatch, entries, statuses,
            [&filled](const TerrainWorkEntry&, const std::size_t index) {
                filled.push_back(index);
            }) != 0U ||
        filled != std::vector<std::size_t>{0} || dispatch.entry_index != 3U ||
        dispatch.status_index != 1U) {
        return 1;
    }

    TerrainWorkDispatchState batch_dispatch{};
    const std::vector<TerrainWorkEntry> batch_entries{
        {1, 1, 0, 1}, {2, 2, 0, 1}, {0, 0, 0, 1}};
    std::vector<std::size_t> marked;
    std::vector<std::size_t> copied;
    const auto batch_result = process_terrain_work_batch(
        batch_dispatch, batch_entries, {0, 0},
        [&marked](const TerrainWorkEntry& entry, const std::size_t index) {
            marked.push_back(index);
            return entry.width == 2U;
        },
        [&copied](const TerrainWorkEntry& entry) {
            copied.push_back(entry.width);
        });
    if (!batch_result.copied || !batch_result.map_dirty ||
        marked != std::vector<std::size_t>{0, 1} ||
        copied != std::vector<std::size_t>{2U} ||
        batch_dispatch.entry_index != 1U) {
        return 2;
    }

    std::vector<std::uint16_t> source(20U);
    for (std::size_t index = 0; index < source.size(); ++index) {
        source[index] = static_cast<std::uint16_t>(index + 10U);
    }
    std::vector<std::uint16_t> destination(20U, 0U);
    fill_terrain_work_region(destination, 2U, 2U, 5U, 0U);
    if (destination[0] != 2U || destination[1] != 2U ||
        destination[5] != 2U || destination[6] != 2U ||
        destination[2] != 0U) {
        return 2;
    }

    std::fill(destination.begin(), destination.end(), 0U);
    copy_terrain_work_region(destination, source, 2U, 2U, 5U, 10U, 1U);
    if (destination[10] != 11U || destination[11] != 12U ||
        destination[15] != 16U || destination[16] != 17U) {
        return 2;
    }

    std::fill(destination.begin(), destination.end(), 0U);
    copy_terrain_work_region_rows(
        destination, source, {2, 2, 5, 1, 6, -1},
        [](const TerrainWorkRegionRequest&) {
            return TerrainWorkRegionRequest{2, 1, 5, 0, 10, 0};
        });
    if (destination[10] != 10U || destination[11] != 11U ||
        destination[15] != 0U) {
        return 2;
    }

    std::uint8_t marker = 0U;
    const TerrainBoundary boundary{0x0100U, 0x0400U, 1U, 2U};
    if (!mark_terrain_boundary_if_inside(24U, 16U, boundary, marker) ||
        marker != 0xFFU ||
        mark_terrain_boundary_if_inside(0U, 16U, boundary, marker)) {
        return 3;
    }

    int regular_calls = 0;
    int alternate_calls = 0;
    if (!mark_terrain_boundary_for_mode(
            true,
            [&regular_calls] {
                ++regular_calls;
                return false;
            },
            [&alternate_calls] {
                ++alternate_calls;
                return true;
            }) ||
        regular_calls != 1 || alternate_calls != 1 ||
        mark_terrain_boundary_for_mode(false, {}, {}) ) {
        return 4;
    }

    return 0;
}
