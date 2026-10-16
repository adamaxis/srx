#include "shadowrun_ghidra/entities/animation_reset_conversion.hpp"

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    shadowrun::ghidra_conversion::EntityAnimationResetState state{
        17, 0x40, 5};
    shadowrun::ghidra_conversion::reset_entity_animation_state(state);

    if (expect(state.frame_timer == 0) != 0 ||
        expect(state.frame_phase == 0) != 0 ||
        expect(state.render_flags == 0x41) != 0) {
        return 1;
    }

    return 0;
}
