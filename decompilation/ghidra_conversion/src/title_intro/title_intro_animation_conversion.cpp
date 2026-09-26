#include "shadowrun_ghidra/title_intro/title_intro_animation_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

void invoke(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

} // namespace

void advance_title_intro_animation(
    TitleIntroAnimationState& state,
    const TitleIntroAnimationCallbacks& callbacks) {
    state.animation_pass = 1U;
    invoke(callbacks.advance_persistent_sequence);
    invoke(callbacks.advance_motion);
    invoke(callbacks.advance_dual_sprites);
    invoke(callbacks.advance_timed_sprites);
    invoke(callbacks.advance_targeted_sweep);
    invoke(callbacks.advance_linear_sweep_a);
    invoke(callbacks.advance_linear_sweep_b);
    invoke(callbacks.advance_final_projection);
    state.scene_marker = 0;
    state.dirty_flags = static_cast<std::uint8_t>(state.dirty_flags | 0x02U);
}

} // namespace shadowrun::ghidra_conversion
