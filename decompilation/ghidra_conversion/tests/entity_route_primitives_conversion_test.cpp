#include "shadowrun_ghidra/entity_route_primitives_conversion.hpp"

#include <array>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (!expect(compute_obstruction_grid_index(0x1200U, 0x1000U) ==
                0x0209U)) {
        return 1;
    }
    if (!expect(compute_obstruction_grid_index(0x01FFU, 0x003FU) ==
                0U)) {
        return 2;
    }

    if (!expect(derive_entity_axis_relation_flags(10, 20, 10, 20) == 0x03U)) {
        return 3;
    }
    if (!expect(derive_entity_axis_relation_flags(11, 19, 10, 20) == 0x02U)) {
        return 4;
    }
    if (!expect(derive_entity_axis_relation_flags(9, 21, 10, 20) == 0x01U)) {
        return 5;
    }

    const std::array<std::uint8_t, 6> occupied{{1, 1, 0, 1, 1, 1}};
    if (!expect(find_first_available_entity_slot(occupied) == 11U)) {
        return 6;
    }
    const std::array<std::uint8_t, 6> all_occupied{{1, 1, 1, 1, 1, 1}};
    if (!expect(find_first_available_entity_slot(all_occupied) == 15U)) {
        return 7;
    }
    const std::array<std::uint8_t, 6> non_active_flags{{2, 4, 8, 0, 0, 0}};
    if (!expect(find_first_available_entity_slot(non_active_flags) == 9U)) {
        return 8;
    }

    return 0;
}
