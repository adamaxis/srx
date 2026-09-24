#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct TitleIntroAnimationState {
    std::uint16_t animation_pass = 0;
    std::uint16_t scene_marker = 0;
    std::uint8_t dirty_flags = 0;
};

struct TitleIntroAnimationCallbacks {
    std::function<void()> advance_persistent_sequence;
    std::function<void()> advance_motion;
    std::function<void()> advance_dual_sprites;
    std::function<void()> advance_timed_sprites;
    std::function<void()> advance_targeted_sweep;
    std::function<void()> advance_linear_sweep_a;
    std::function<void()> advance_linear_sweep_b;
    std::function<void()> advance_final_projection;
};

// ROM 0x5A1EA: run one source-ordered pass over the title animation services.
// The child animation implementations remain typed callback boundaries.
void advance_title_intro_animation(
    TitleIntroAnimationState& state,
    const TitleIntroAnimationCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
