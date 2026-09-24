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

    return 0;
}
