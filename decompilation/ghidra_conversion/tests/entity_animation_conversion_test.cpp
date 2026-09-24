#include "shadowrun_ghidra/entity_animation_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityAnimationState steady{3U, 1, 2, 8, false};
    advance_entity_animation_accumulator(steady);
    if (!expect(steady.frame == 3U && steady.accumulator == 3 &&
                !steady.redraw_requested)) {
        return 1;
    }

    EntityAnimationState stepped{7U, 6, 3, 8, false};
    advance_entity_animation_accumulator(stepped);
    if (!expect(stepped.frame == 8U && stepped.accumulator == 1 &&
                stepped.redraw_requested)) {
        return 2;
    }

    // The source performs one subtraction per call rather than looping until
    // the accumulator is below the period.
    EntityAnimationState one_step{0xFFFFU, 12, 20, 8, false};
    advance_entity_animation_accumulator(one_step);
    if (!expect(one_step.frame == 0U && one_step.accumulator == 24 &&
                one_step.redraw_requested)) {
        return 3;
    }

    return 0;
}
