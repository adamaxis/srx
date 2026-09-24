#include "shadowrun_ghidra/terrain_map_conversion.hpp"

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
    bool post_processed = false;
    rebuild_terrain_collision_map(
        map, true,
        {{0x0020, 0x0060, 0x0040, 0x0060, 7}},
        [&post_processed](auto& terrain_map) {
            post_processed = true;
            replace_nth_terrain_marker(terrain_map, 7, 0xDA, 1);
        });
    if (!expect(post_processed) ||
        !expect(map[map_index(0x20, 0x40)] == 7 &&
                map[map_index(0x40, 0x40)] == 0xDA &&
                map[map_index(0x60, 0x60)] == 7) ||
        !expect(map[map_index(0x00, 0x00)] == 0)) {
        return 1;
    }

    const auto before = map;
    rebuild_terrain_collision_map(
        map, false, {{0, 0, 0, 0, 1}},
        [](auto&) { std::abort(); });
    if (!expect(map == before)) {
        return 2;
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
        return 3;
    }

    return 0;
}
