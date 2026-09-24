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

    std::vector<std::uint8_t> obstruction_map(32, 0U);
    obstruction_map[12] = 1U;
    const auto horizontal_obstruction =
        find_first_occupied_obstruction_tile(10, 2, obstruction_map);
    if (!expect(horizontal_obstruction.has_value() &&
                horizontal_obstruction->tile_index == 12 &&
                horizontal_obstruction->step == 1)) {
        return 3;
    }
    std::vector<std::uint8_t> vertical_map(128, 0U);
    vertical_map[6] = 1U;
    const auto vertical_obstruction =
        find_first_occupied_obstruction_tile(70, 0, vertical_map);
    if (!expect(vertical_obstruction.has_value() &&
                vertical_obstruction->tile_index == 6 &&
                vertical_obstruction->step == -64)) {
        return 4;
    }
    if (!expect(!find_first_occupied_obstruction_tile(
                    10, 2, std::vector<std::uint8_t>(32, 0U))
                    .has_value())) {
        return 5;
    }

    std::vector<std::uint8_t> strip_map(128, 0U);
    strip_map[75] = 1U;
    const auto strip = probe_obstruction_strip(10, 2, 4, strip_map);
    if (!expect(strip.starting_tile == 10 && strip.final_tile == 12 &&
                !strip.left_map)) {
        return 6;
    }
    const auto edge_strip = probe_obstruction_strip(0, 0, 2, strip_map);
    if (!expect(edge_strip.starting_tile == 0 &&
                edge_strip.final_tile == -64 && edge_strip.left_map)) {
        return 7;
    }

    if (!expect(!can_bypass_obstruction(
                    10, 2, 4, std::vector<std::uint8_t>(128, 0U)))) {
        return 8;
    }
    std::vector<std::uint8_t> blocked_route(128, 1U);
    blocked_route[10] = 0U;
    if (!expect(can_bypass_obstruction(10, 2, 4, blocked_route))) {
        return 9;
    }

    std::vector<std::uint8_t> horizontal_path(128, 0U);
    horizontal_path[12] = 1U;
    if (!expect(is_obstruction_path_blocked(10, 13, horizontal_path))) {
        return 10;
    }
    std::vector<std::uint8_t> vertical_path(128, 0U);
    vertical_path[10] = 1U;
    if (!expect(is_obstruction_path_blocked(10, 74, vertical_path))) {
        return 11;
    }
    if (!expect(!is_obstruction_path_blocked(
                    10, 13, std::vector<std::uint8_t>(128, 0U)))) {
        return 12;
    }

    const auto north = project_heading_position(1000, 2000, 0x0000U);
    if (!expect(north.world_x == 1000 && north.world_y == 1872)) {
        return 13;
    }
    const auto southeast = project_heading_position(1000, 2000, 0x6000U);
    if (!expect(southeast.world_x == 1090 && southeast.world_y == 2090)) {
        return 14;
    }

    if (!expect(compute_obstruction_grid_index(0x1200U, 0x1000U) ==
                0x0209U)) {
        return 15;
    }
    if (!expect(compute_obstruction_grid_index(0x01FFU, 0x003FU) ==
                0U)) {
        return 16;
    }

    const auto same_tile = decode_obstruction_tile_projection(0, 0);
    if (!expect(same_tile.world_x == 0x0100U &&
                same_tile.world_y == 0x0100U && same_tile.direction == 6)) {
        return 17;
    }
    const auto left_tile = decode_obstruction_tile_projection(1, 0);
    if (!expect(left_tile.world_x == 0x0300U &&
                left_tile.world_y == 0x0100U && left_tile.direction == 2)) {
        return 18;
    }
    const auto lower_tile = decode_obstruction_tile_projection(0x40, 0);
    if (!expect(lower_tile.world_y == 0x0300U && lower_tile.direction == 4)) {
        return 19;
    }
    const auto upper_tile = decode_obstruction_tile_projection(0, 0x40);
    if (!expect(upper_tile.direction == 0)) {
        return 20;
    }

    if (!expect(derive_entity_axis_relation_flags(10, 20, 10, 20) == 0x03U)) {
        return 21;
    }
    if (!expect(derive_entity_axis_relation_flags(11, 19, 10, 20) == 0x02U)) {
        return 22;
    }
    if (!expect(derive_entity_axis_relation_flags(9, 21, 10, 20) == 0x01U)) {
        return 23;
    }

    const std::array<std::uint8_t, 6> occupied{{1, 1, 0, 1, 1, 1}};
    if (!expect(find_first_available_entity_slot(occupied) == 11U)) {
        return 24;
    }
    const std::array<std::uint8_t, 6> all_occupied{{1, 1, 1, 1, 1, 1}};
    if (!expect(find_first_available_entity_slot(all_occupied) == 15U)) {
        return 25;
    }
    const std::array<std::uint8_t, 6> non_active_flags{{2, 4, 8, 0, 0, 0}};
    if (!expect(find_first_available_entity_slot(non_active_flags) == 9U)) {
        return 26;
    }

    return 0;
}
