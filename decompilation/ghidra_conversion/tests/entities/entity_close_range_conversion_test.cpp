#include "shadowrun_ghidra/entities/entity_close_range_conversion.hpp"

#include <cstdint>

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityCloseRangeState state{0xF7U, 0x12U, 0x12345678U};
    if (!apply_close_range_entity_state(state, 0x01DFU) ||
        state.flags != 0x84U || state.secondary_state != 0U ||
        state.continuation != 0xB45CU) {
        return 1;
    }

    const auto previous = state;
    if (apply_close_range_entity_state(state, 0x01E0U) ||
        state.flags != previous.flags ||
        state.secondary_state != previous.secondary_state ||
        state.continuation != previous.continuation) {
        return 2;
    }

    bool called = false;
    apply_entity_close_range_state([&called] { called = true; });
    if (!called) {
        return 3;
    }

    return 0;
}
