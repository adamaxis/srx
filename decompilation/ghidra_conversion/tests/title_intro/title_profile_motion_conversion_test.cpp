#include "shadowrun_ghidra/title_intro/title_profile_motion_conversion.hpp"

#include <optional>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleProfileMotionState phase{};
    phase.motion_mode = 1U;
    phase.profile_frame_delay = 0;
    phase.profile_phase = 0U;
    phase.profile_x = 0x100U;
    phase.profile_y = 0x200U;
    phase.moving_x = 0x1340U;
    phase.moving_y = 0x1000U;
    phase.movement_countdown = 2U;
    phase.secondary_frame = 2U;
    int builds = 0;
    TitleProfileMotionCallbacks callbacks{};
    callbacks.resolve_profile_sprite =
        [](const std::uint16_t value)
        -> std::optional<TitleProfileMotionSprite> {
        return TitleProfileMotionSprite{0x1234U, 2, -3};
    };
    callbacks.resolve_movement_easing =
        [](const std::uint16_t) -> std::optional<std::int16_t> { return 0; };
    callbacks.resolve_moving_sprite =
        [](const std::uint16_t) -> std::optional<std::uint32_t> {
        return 0x5678U;
    };
    callbacks.build_sprite = [&](const TitleProfileMotionSpriteBuild& sprite) {
        if (sprite.moving_sprite) {
            if (sprite.x != 0x134U || sprite.y != 0x0FFU ||
                sprite.phase != 0U) {
                builds += 100;
            }
        } else if (sprite.x != 0x102U || sprite.y != 0x1FDU ||
                   sprite.phase != 1U || sprite.sprite_data_token != 0U) {
            builds += 100;
        }
        ++builds;
    };
    if (advance_title_profile_motion(phase, callbacks) !=
            TitleProfileMotionAction::rendered ||
        phase.profile_phase != 1U || phase.profile_frame_delay != 1 ||
        builds != 2) {
        return 1;
    }

    TitleProfileMotionState finish{};
    finish.motion_mode = 2U;
    finish.movement_countdown = 0U;
    finish.pending_selection = true;
    int refreshed = 0;
    callbacks = {};
    callbacks.refresh_menu_selection_or_prepare = [&] { ++refreshed; };
    if (advance_title_profile_motion(finish, callbacks) !=
            TitleProfileMotionAction::finalized ||
        finish.motion_mode != 0U || finish.transition_state != 0U ||
        finish.completion_state != 1U || refreshed != 1) {
        return 2;
    }
    return 0;
}
