#include "shadowrun_ghidra/title_profile_motion_conversion.hpp"

#include <cstdint>

namespace shadowrun::ghidra_conversion {
namespace {

bool decrement_word_until_negative(std::uint16_t& value) {
    value = static_cast<std::uint16_t>(value - 1U);
    return (value & 0x8000U) != 0U;
}

bool decrement_byte_until_negative(std::int8_t& value) {
    value = static_cast<std::int8_t>(value - 1);
    return value < 0;
}

bool build_sprite(const TitleProfileMotionSpriteBuild& sprite,
                  const TitleProfileMotionCallbacks& callbacks) {
    if (callbacks.build_sprite) {
        callbacks.build_sprite(sprite);
    }
    return true;
}

} // namespace

TitleProfileMotionAction advance_title_profile_motion(
    TitleProfileMotionState& state,
    const TitleProfileMotionCallbacks& callbacks) {
    bool rendered = false;
    if (state.motion_mode == 1U) {
        if (decrement_byte_until_negative(state.profile_frame_delay)) {
            state.profile_frame_delay = 1;
            ++state.profile_phase;
            if (state.profile_phase >= 0x0DU) {
                state.motion_mode = 2U;
                state.moving_x = 0x1340U;
                state.moving_y = 0x0FF0U;
                state.secondary_frame = 0x000FU;
                state.movement_countdown = 0x003CU;
                state.secondary_phase = 0U;
            } else {
                const auto sprite = callbacks.resolve_profile_sprite
                                         ? callbacks.resolve_profile_sprite(
                                               state.profile_phase)
                                         : std::nullopt;
                if (!sprite.has_value()) {
                    return TitleProfileMotionAction::sprite_data_unresolved;
                }
                rendered = build_sprite(
                    TitleProfileMotionSpriteBuild{
                        sprite->source_table_token, state.profile_phase,
                        static_cast<std::uint16_t>(
                            state.profile_x + sprite->offset_x),
                        static_cast<std::uint16_t>(
                            state.profile_y + sprite->offset_y), 0x6000U,
                        0U, false},
                    callbacks);
            }
        }
    }

    if (decrement_word_until_negative(state.movement_countdown)) {
        state.motion_mode = 0U;
        state.transition_state = 0U;
        if (state.pending_selection) {
            state.completion_state = 1U;
        }
        if (callbacks.refresh_menu_selection_or_prepare) {
            callbacks.refresh_menu_selection_or_prepare();
        }
        return TitleProfileMotionAction::finalized;
    }

    if (decrement_word_until_negative(state.secondary_frame)) {
        state.secondary_frame = 0x000FU;
        ++state.secondary_phase;
        if (state.secondary_phase >= 4U) {
            state.motion_mode = 0U;
            state.transition_state = 0U;
            if (state.pending_selection) {
                state.completion_state = 1U;
            }
            if (callbacks.refresh_menu_selection_or_prepare) {
                callbacks.refresh_menu_selection_or_prepare();
            }
            return TitleProfileMotionAction::finalized;
        }
    }

    const auto elapsed_step = static_cast<std::uint16_t>(
        0x003CU - state.movement_countdown);
    const auto easing = callbacks.resolve_movement_easing
                            ? callbacks.resolve_movement_easing(elapsed_step)
                            : std::nullopt;
    if (!easing.has_value()) {
        return TitleProfileMotionAction::easing_unresolved;
    }
    const auto scaled_bits = static_cast<std::uint32_t>(
        static_cast<std::int64_t>(*easing) * 0x58LL);
    const auto shifted_bits = scaled_bits << 6U;
    const auto offset = static_cast<std::int16_t>(shifted_bits >> 16U);
    state.moving_x = static_cast<std::uint16_t>(0x1340 - offset);
    state.moving_y = static_cast<std::uint16_t>(state.moving_y - 0x000AU);

    const auto moving_sprite = callbacks.resolve_moving_sprite
                                   ? callbacks.resolve_moving_sprite(
                                         state.secondary_phase)
                                   : std::nullopt;
    if (!moving_sprite.has_value()) {
        return TitleProfileMotionAction::sprite_data_unresolved;
    }
    rendered = build_sprite(
        TitleProfileMotionSpriteBuild{
            0x0001BE3EU, state.secondary_phase,
            static_cast<std::uint16_t>(state.moving_x >> 4U),
            static_cast<std::uint16_t>(state.moving_y >> 4U), 0x6000U,
            *moving_sprite, true},
        callbacks) ||
               rendered;
    return rendered ? TitleProfileMotionAction::rendered
                    : TitleProfileMotionAction::updated;
}

} // namespace shadowrun::ghidra_conversion
