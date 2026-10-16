#include "shadowrun_ghidra/title_intro/title_effect_motion_conversion.hpp"

#include <array>

namespace shadowrun::ghidra_conversion {
namespace {

constexpr std::array<std::uint32_t, 12> kTitleEffectMotionTable1C926{
    0x0001D772U, 0x0001D78CU, 0x0001D7A6U, 0x0001D7C0U,
    0x0001D7A6U, 0x0001D78CU, 0x0001D772U, 0x0001D78CU,
    0x0001D7A6U, 0x0001D7C0U, 0x0001D7A6U, 0x0001D78CU};
constexpr std::array<std::uint32_t, 12> kTitleEffectMotionTable1C956{
    0x0001D7EAU, 0x0001D804U, 0x0001D81EU, 0x0001D838U,
    0x0001D81EU, 0x0001D804U, 0x0001D7EAU, 0x0001D804U,
    0x0001D81EU, 0x0001D838U, 0x0001D81EU, 0x0001D804U};

} // namespace

std::optional<std::uint32_t> resolve_title_effect_motion_sprite_data(
    const std::uint16_t effect_mode, const std::uint16_t frame,
    const std::uint16_t position_x) {
    if (frame >= kTitleEffectMotionTable1C926.size()) {
        return std::nullopt;
    }

    // ROM 0x1C8D8..0x1C904 selects 0x1C956 for (mode != 0) ==
    // (position_x >= 0x120), and 0x1C926 otherwise.
    const bool right_side = position_x >= 0x0120U;
    const bool use_right_table = (effect_mode != 0U) == right_side;
    return use_right_table ? kTitleEffectMotionTable1C956[frame]
                           : kTitleEffectMotionTable1C926[frame];
}

TitleEffectMotionAction advance_title_effect_motion(
    TitleEffectMotionState& state,
    const TitleEffectMotionCallbacks& callbacks) {
    state.frame_delay = static_cast<std::uint16_t>(state.frame_delay - 1U);
    if ((state.frame_delay & 0x8000U) == 0U) {
        return TitleEffectMotionAction::waiting;
    }
    state.frame_delay = 3U;
    ++state.frame;
    if (state.frame >= 0x0CU) {
        if (state.position_x < 0x0120U) {
            state.secondary_behavior_descriptor = 0x0001C288U;
            state.secondary_stage = 0U;
            if (callbacks.publish_behavior_descriptor) {
                callbacks.publish_behavior_descriptor(
                    TitleEffectMotionRecordSlot::secondary, 0x0001C288U);
            }
        } else {
            state.primary_behavior_descriptor = 0x0001C28EU;
            state.primary_stage = 0U;
            if (callbacks.publish_behavior_descriptor) {
                callbacks.publish_behavior_descriptor(
                    TitleEffectMotionRecordSlot::primary, 0x0001C28EU);
            }
        }
        state.transition_state = 0U;
        if (callbacks.run_menu_selection_service) {
            callbacks.run_menu_selection_service();
        }
        state.menu_state = 0U;
        return TitleEffectMotionAction::finalized;
    }

    const auto sprite = callbacks.resolve_sprite_data
                            ? callbacks.resolve_sprite_data(
                                  state.effect_mode, state.frame,
                                  state.position_x)
                            : resolve_title_effect_motion_sprite_data(
                                  state.effect_mode, state.frame,
                                  state.position_x);
    if (!sprite.has_value()) {
        return TitleEffectMotionAction::sprite_data_unresolved;
    }
    if (callbacks.build_sprite) {
        callbacks.build_sprite(*sprite, state.position_x, state.position_y,
                               0x2000U);
    }
    return TitleEffectMotionAction::rendered;
}

} // namespace shadowrun::ghidra_conversion
