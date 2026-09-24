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

    return 0;
}
