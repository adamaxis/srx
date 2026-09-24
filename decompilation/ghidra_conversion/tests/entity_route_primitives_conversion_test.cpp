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

    std::vector<std::uint8_t> alternate_route_map(128, 0U);
    alternate_route_map[74] = 1U;
    alternate_route_map[75] = 1U;
    if (!expect(find_alternate_route_tile(
                    10, 2, 4, 63, 1, alternate_route_map) == 12)) {
        return 27;
    }
    alternate_route_map[74] = 0U;
    if (!expect(find_alternate_route_tile(
                    10, 2, 4, 63, 1, alternate_route_map) == 74)) {
        return 28;
    }
    alternate_route_map[74] = 1U;
    alternate_route_map[11] = 1U;
    if (!expect(find_alternate_route_tile(
                    10, 2, 4, 63, 1, alternate_route_map) == 10)) {
        return 29;
    }
    std::vector<std::uint8_t> boundary_route_map(128, 0U);
    boundary_route_map[74] = 1U;
    boundary_route_map[75] = 1U;
    boundary_route_map[76] = 1U;
    if (!expect(find_alternate_route_tile(
                    10, 2, 4, 12, 1, boundary_route_map) == 12)) {
        return 30;
    }

    const auto strip_target = find_rotating_route_strip_target(
        0x0200U, 0x0100U, 0x4000U, std::vector<std::uint8_t>(128, 0U));
    if (!expect(strip_target.has_value() && strip_target.value() == 2)) {
        return 40;
    }
    std::vector<std::uint8_t> fully_blocked_strip(128, 1U);
    fully_blocked_strip[1] = 0U;
    if (!expect(!find_rotating_route_strip_target(
                     0x0200U, 0x0100U, 0x4000U, fully_blocked_strip)
                     .has_value())) {
        return 41;
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

    std::vector<std::uint8_t> world_map(256, 0U);
    if (!expect(!is_entity_path_blocked(
                    0x0200U, 0x0100U, 0x0400U, 0x0100U, world_map))) {
        return 31;
    }
    const auto target_tile = compute_obstruction_grid_index(0x0400U, 0x0100U);
    if (!expect(target_tile < world_map.size())) {
        return 32;
    }
    world_map[target_tile - 1U] = 1U;
    if (!expect(is_entity_path_blocked(
                    0x0200U, 0x0100U, 0x0400U, 0x0100U, world_map))) {
        return 33;
    }

    const auto no_route_request = update_entity_axis_relation_state(
        10, 20, 0, 10, 0x00U, 0x0000, 0x40U);
    if (!expect(no_route_request.axis_flags == 0x00U &&
                no_route_request.movement_flags == 0x40U &&
                !no_route_request.route_advance_requested)) {
        return 34;
    }
    const auto route_request = update_entity_axis_relation_state(
        10, 20, 20, 30, 0x00U, 0x0000, 0x40U);
    if (!expect(route_request.axis_flags == 0x0CU &&
                route_request.movement_flags == 0x42U &&
                route_request.route_advance_requested)) {
        return 35;
    }
    const auto inverted_axes = update_entity_axis_relation_state(
        20, 10, 10, 0, 0x03U, 0x0002, 0x00U);
    if (!expect(inverted_axes.axis_flags == 0x0FU &&
                inverted_axes.movement_flags == 0x00U &&
                !inverted_axes.route_advance_requested)) {
        return 36;
    }

    return 0;
}
