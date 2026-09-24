#include "shadowrun_ghidra/entity_coordinate_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityCoordinateTarget descending{0x1000U, 0x0000U};
    approach_entity_fixed_point_target(descending);
    if (!expect(descending.current == 0x0800U)) {
        return 1;
    }

    EntityCoordinateTarget ascending{0x0000U, 0x1000U};
    approach_entity_fixed_point_target(ascending);
    if (!expect(ascending.current == 0x0800U)) {
        return 2;
    }

    EntityCoordinateTarget snap_down{0x0800U, 0x0000U};
    approach_entity_fixed_point_target(snap_down);
    EntityCoordinateTarget snap_up{0x0000U, 0x0800U};
    approach_entity_fixed_point_target(snap_up);
    if (!expect(snap_down.current == 0x0000U &&
                snap_up.current == 0x0800U)) {
        return 3;
    }

    EntityCoordinateTarget unchanged{0x1234U, 0x1234U};
    approach_entity_fixed_point_target(unchanged);
    if (!expect(unchanged.current == 0x1234U)) {
        return 4;
    }

    FieldBoundaryMotionState lower_edge{0x2600U, 0x1200U};
    apply_mode_five_boundary_motion(lower_edge, 5U);
    if (!expect(lower_edge.y == 0x11F8U)) {
        return 5;
    }

    FieldBoundaryMotionState upper_edge{0x3C00U, 0x1F7FU};
    apply_mode_five_boundary_motion(upper_edge, 5U);
    if (!expect(upper_edge.y == 0x1F87U)) {
        return 6;
    }

    FieldBoundaryMotionState outside_window{0x25FFU, 0x1200U};
    apply_mode_five_boundary_motion(outside_window, 5U);
    if (!expect(outside_window.y == 0x1200U)) {
        return 7;
    }

    FieldBoundaryMotionState wrong_mode{0x2600U, 0x1200U};
    apply_mode_five_boundary_motion(wrong_mode, 4U);
    if (!expect(wrong_mode.y == 0x1200U)) {
        return 8;
    }

    std::int32_t probed_x = 0;
    std::int32_t probed_y = 0;
    const auto probe = probe_entity_world_position(
        EntityFixedPointPosition{0x00123456,
                                 static_cast<std::int32_t>(0xFFF00000U),
                                 2,
                                 -3},
        [&probed_x, &probed_y](const std::int32_t x, const std::int32_t y) {
            probed_x = x;
            probed_y = y;
            return static_cast<std::uint16_t>(0x007FU);
        });
    if (!expect(probe.world_x == 0x00123656 &&
                probe.world_y == static_cast<std::int32_t>(0xFFEFFD00U) &&
                probe.terrain_result == 0x007FU && probed_x == probe.world_x &&
                probed_y == probe.world_y)) {
        return 9;
    }

    const auto north = derive_entity_heading_offsets(0x4000U);
    if (!expect(north.horizontal == 0 && north.vertical == 24575)) {
        return 10;
    }
    const auto diagonal = derive_entity_heading_offsets(0x0200U);
    if (!expect(diagonal.horizontal == -24546 && diagonal.vertical == 1205)) {
        return 11;
    }
    const auto east = derive_entity_heading_offsets(0x8000U);
    if (!expect(east.horizontal == 24575 && east.vertical == 0)) {
        return 12;
    }

    return 0;
}
