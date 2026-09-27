#include "shadowrun_ghidra/field/terrain_map_conversion.hpp"

#include <array>
#include <cstdlib>
#include <cstdint>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

std::size_t map_index(
    const std::uint16_t x,
    const std::uint16_t y) {
    return static_cast<std::size_t>(
        ((((static_cast<std::uint32_t>(y) & 0xFFE0U) << 1U) +
          (x >> 5U)) &
         0x0FFFU));
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::array<std::uint8_t, 0x1000> map{};
    map.fill(0xEE);
    bool rebuild_requested = true;
    bool post_processed = false;
    rebuild_terrain_collision_map(
        map, rebuild_requested,
        {{0x0020, 0x0060, 0x0040, 0x0060, 7}},
        [&post_processed](auto& terrain_map) {
            post_processed = true;
            replace_nth_terrain_marker(terrain_map, 7, 0xDA, 1);
        });
    if (!expect(!rebuild_requested) || !expect(post_processed) ||
        !expect(map[map_index(0x20, 0x40)] == 7 &&
                map[map_index(0x40, 0x40)] == 0xDA &&
                map[map_index(0x60, 0x60)] == 7) ||
        !expect(map[map_index(0x00, 0x00)] == 0)) {
        return 1;
    }

    const auto before = map;
    rebuild_requested = false;
    rebuild_terrain_collision_map(
        map, rebuild_requested, {{0, 0, 0, 0, 1}},
        [](auto&) { std::abort(); });
    if (!expect(map == before)) {
        return 2;
    }

    TerrainMarkerReplacementState marker_state{};
    marker_state.field_mode = 2;
    marker_state.effect_context_active = 1;
    marker_state.selected_effect_index = 4;
    marker_state.active_effect_index = 4;
    marker_state.marker_ordinal = 1;
    std::array<std::uint8_t, 0x1000> gated_markers{};
    gated_markers[2] = 0xDA;
    gated_markers[5] = 0xDA;
    if (!expect(replace_nth_terrain_marker(gated_markers, marker_state)) ||
        !expect(gated_markers[2] == 0xDA && gated_markers[5] == 0x9A)) {
        return 3;
    }
    marker_state.world_effect_active = true;
    if (!expect(!replace_nth_terrain_marker(gated_markers, marker_state))) {
        return 4;
    }

    std::array<std::uint8_t, 0x1000> markers{};
    markers.fill(0);
    markers[3] = 0xDA;
    markers[7] = 0xDA;
    markers[9] = 0xDA;
    replace_nth_terrain_marker(markers, 0xDA, 0x9A, 0);
    replace_nth_terrain_marker(markers, 0xDA, 0x9B, 1);
    if (!expect(markers[3] == 0x9A &&
                markers[7] == 0xDA &&
                markers[9] == 0x9B)) {
        return 5;
    }

    std::vector<std::uint16_t> work_map(20U, 0U);
    fill_terrain_work_region(work_map, 3U, 2U, 5U, 1U);
    if (!expect(work_map[1] == 2U && work_map[2] == 2U &&
                work_map[3] == 2U && work_map[6] == 2U &&
                work_map[7] == 2U && work_map[8] == 2U &&
                work_map[0] == 0U && work_map[9] == 0U)) {
        return 6;
    }

    fill_terrain_work_region(work_map, 4U, 4U, 5U, 19U);
    if (!expect(work_map[19] == 2U)) {
        return 7;
    }

    std::vector<std::uint16_t> source(20U, 0U);
    for (std::size_t index = 0; index < source.size(); ++index) {
        source[index] = static_cast<std::uint16_t>(index + 10U);
    }
    std::vector<std::uint16_t> destination(20U, 0U);
    copy_terrain_work_region(destination, source, 3U, 2U, 5U, 6U, 1U);
    if (!expect(destination[6] == 11U && destination[7] == 12U &&
                destination[8] == 13U && destination[11] == 16U &&
                destination[12] == 17U && destination[13] == 18U)) {
        return 8;
    }

    std::vector<std::uint16_t> layer_destination(4U, 0U);
    copy_terrain_layer(layer_destination, source, 4U);
    if (!expect(layer_destination[0] == 10U &&
                layer_destination[3] == 13U)) {
        return 9;
    }

    return 0;
}
