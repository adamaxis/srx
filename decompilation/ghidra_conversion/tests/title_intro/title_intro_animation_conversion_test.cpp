#include "shadowrun_ghidra/title_intro/title_intro_animation_conversion.hpp"

#include <string>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleIntroAnimationState state{0U, 7U, 0x04U};
    std::vector<std::string> order;
    TitleIntroAnimationCallbacks callbacks{};
    callbacks.advance_persistent_sequence =
        [&] { order.emplace_back("persistent"); };
    callbacks.advance_motion = [&] { order.emplace_back("motion"); };
    callbacks.advance_dual_sprites =
        [&] { order.emplace_back("dual"); };
    callbacks.advance_timed_sprites =
        [&] { order.emplace_back("timed"); };
    callbacks.advance_targeted_sweep =
        [&] { order.emplace_back("targeted"); };
    callbacks.advance_linear_sweep_a =
        [&] { order.emplace_back("linear_a"); };
    callbacks.advance_linear_sweep_b =
        [&] { order.emplace_back("linear_b"); };
    callbacks.advance_final_projection =
        [&] { order.emplace_back("final"); };

    advance_title_intro_animation(state, callbacks);
    const std::vector<std::string> expected{
        "persistent", "motion", "dual", "timed",
        "targeted", "linear_a", "linear_b", "final"};
    if (state.animation_pass != 1U || state.scene_marker != 0U ||
        state.dirty_flags != 0x06U || order != expected) {
        return 1;
    }
    return 0;
}
