#include "shadowrun_ghidra/entity_position_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityPositionState state{0x1111U, 0x2222U, 0xFFU};
    commit_entity_position(state, 0x3456U, 0x789AU);
    if (!expect(state.x == 0x3456U && state.y == 0x789AU &&
                state.transition_flags == 0xFEU)) {
        return 1;
    }

    commit_entity_position(state, 0x0000U, 0xFFFFU);
    if (!expect(state.x == 0x0000U && state.y == 0xFFFFU &&
                state.transition_flags == 0xFEU)) {
        return 2;
    }

    return 0;
}
