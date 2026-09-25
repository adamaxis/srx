#include "shadowrun_ghidra/title_sweep_animation_conversion.hpp"

#include <optional>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleSweepAnimationState sweep{};
    sweep.frame_delay = 0;
    sweep.phase = 0x03U;
    sweep.base_x = 0x100U;
    sweep.base_y = 0x200U;
    int builds = 0;
    TitleSweepAnimationCallbacks callbacks{};
    callbacks.resolve_frame =
        [](const std::uint16_t phase) -> std::optional<TitleSweepFrame> {
        return TitleSweepFrame{0x1C6BCU, 6, 3, 0x4321U + phase};
    };
    callbacks.build_sprite = [&](const TitleSweepSpriteBuild& sprite) {
        if (sprite.phase != 4U || sprite.y != 0x200U ||
            sprite.build_flags != 0x6000U ||
            (sprite.x != 0x106U && sprite.x != 0x0FAU)) {
            builds += 100;
        }
        ++builds;
    };
    if (advance_title_sweep_animation(sweep, callbacks) !=
            TitleSweepAnimationAction::rendered ||
        sweep.phase != 4U || sweep.frame_delay != 3 || builds != 2) {
        return 1;
    }

    sweep.record_flags = 0x08U;
    sweep.frame_delay = 0;
    builds = 0;
    if (advance_title_sweep_animation(sweep, callbacks) !=
            TitleSweepAnimationAction::waiting || builds != 0) {
        return 2;
    }

    TitleSweepAnimationState finish{};
    finish.phase = 0x1FU;
    finish.frame_delay = 0;
    finish.record_flags = 0xFFU;
    finish.transition_state = 0x1234U;
    int finalized = 0;
    callbacks = {};
    callbacks.attach_behavior_descriptor_b = [&] { ++finalized; };
    callbacks.return_to_title_state = [&] { ++finalized; };
    if (advance_title_sweep_animation(finish, callbacks) !=
            TitleSweepAnimationAction::finalized ||
        finish.record_flags != 0xF6U || finish.transition_state != 0U ||
        finalized != 2) {
        return 3;
    }
    return 0;
}
