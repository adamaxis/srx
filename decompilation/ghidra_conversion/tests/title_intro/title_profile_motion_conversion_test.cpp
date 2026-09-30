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
                sprite.phase != 0U || sprite.sprite_data_token != 0x5678U) {
                builds += 100;
            }
        } else if (sprite.x != 0x102U || sprite.y != 0x1FDU ||
                   sprite.phase != 1U ||
                   sprite.sprite_data_token != 0x1234U) {
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

    const auto phase_zero = resolve_title_profile_phase_sprite(0U);
    const auto phase_five = resolve_title_profile_phase_sprite(5U);
    const auto moving_three = resolve_title_profile_moving_sprite_data(3U);
    if (!phase_zero.has_value() || phase_zero->source_table_token != 0x1D74AU ||
        phase_zero->offset_x != 0 || phase_zero->offset_y != -48 ||
        !phase_five.has_value() || phase_five->offset_x != 20 ||
        !moving_three.has_value() || *moving_three != 0x1D768U ||
        resolve_title_profile_phase_sprite(13U).has_value() ||
        resolve_title_profile_moving_sprite_data(4U).has_value() ||
        resolve_title_profile_movement_easing(1U) != 1607 ||
        resolve_title_profile_movement_easing(43U) != 0 ||
        resolve_title_profile_movement_easing(44U) != -3211 ||
        resolve_title_profile_movement_easing(60U).has_value()) {
        return 3;
    }
    return 0;
}
