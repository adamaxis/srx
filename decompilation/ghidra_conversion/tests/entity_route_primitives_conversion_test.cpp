#include "shadowrun_ghidra/entity_route_primitives_conversion.hpp"

#include <array>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (!expect(clear_entity_route_flags(0xFFU) == 0xF5U)) {
        return 1;
    }
    if (!expect(clear_entity_route_flags(0x00U) == 0x00U)) {
        return 2;
    }

    const auto north = project_heading_position(1000, 2000, 0x0000U);
    if (!expect(north.world_x == 1000 && north.world_y == 1872)) {
        return 3;
    }
    const auto southeast = project_heading_position(1000, 2000, 0x6000U);
    if (!expect(southeast.world_x == 1090 && southeast.world_y == 2090)) {
        return 4;
    }

    if (!expect(compute_obstruction_grid_index(0x1200U, 0x1000U) ==
                0x0209U)) {
        return 5;
    }
    if (!expect(compute_obstruction_grid_index(0x01FFU, 0x003FU) ==
                0U)) {
        return 6;
    }

    const auto same_tile = decode_obstruction_tile_projection(0, 0);
    if (!expect(same_tile.world_x == 0x0100U &&
                same_tile.world_y == 0x0100U && same_tile.direction == 6)) {
        return 7;
    }
    const auto left_tile = decode_obstruction_tile_projection(1, 0);
    if (!expect(left_tile.world_x == 0x0300U &&
                left_tile.world_y == 0x0100U && left_tile.direction == 2)) {
        return 8;
    }
    const auto lower_tile = decode_obstruction_tile_projection(0x40, 0);
    if (!expect(lower_tile.world_y == 0x0300U && lower_tile.direction == 4)) {
        return 9;
    }
    const auto upper_tile = decode_obstruction_tile_projection(0, 0x40);
    if (!expect(upper_tile.direction == 0)) {
        return 10;
    }

    if (!expect(derive_entity_axis_relation_flags(10, 20, 10, 20) == 0x03U)) {
        return 11;
    }
    if (!expect(derive_entity_axis_relation_flags(11, 19, 10, 20) == 0x02U)) {
        return 12;
    }
    if (!expect(derive_entity_axis_relation_flags(9, 21, 10, 20) == 0x01U)) {
        return 13;
    }

    const std::array<std::uint8_t, 6> occupied{{1, 1, 0, 1, 1, 1}};
    if (!expect(find_first_available_entity_slot(occupied) == 11U)) {
        return 14;
    }
    const std::array<std::uint8_t, 6> all_occupied{{1, 1, 1, 1, 1, 1}};
    if (!expect(find_first_available_entity_slot(all_occupied) == 15U)) {
        return 15;
    }
    const std::array<std::uint8_t, 6> non_active_flags{{2, 4, 8, 0, 0, 0}};
    if (!expect(find_first_available_entity_slot(non_active_flags) == 9U)) {
        return 16;
    }

    return 0;
}
