#include "shadowrun_ghidra/entities/entity_boundary_reposition_conversion.hpp"

#include <cstdint>

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityBoundaryRepositionState state{0U, 0U, 0U};
    const EntityBoundaryRepositionInput input{0x1000U, 0x2000U, 0x1000U,
                                               0x9000U};
    if (!reposition_entity_at_boundary(state, input) ||
        state.flags != 0x40U || state.x != 0xF480U || state.y != 0x4F80U) {
        return 1;
    }

    const auto previous = state;
    if (reposition_entity_at_boundary(state, input) || state.flags != previous.flags ||
        state.x != previous.x || state.y != previous.y) {
        return 2;
    }

    EntityBoundaryRepositionState reverse{0U, 0U, 0U};
    if (!reposition_entity_at_boundary(
            reverse, {0x3000U, 0x4000U, 0xFFFFU, 0xFFFFU}) ||
        reverse.x != 0x5F80U || reverse.y != 0x6F80U) {
        return 3;
    }

    return 0;
}
