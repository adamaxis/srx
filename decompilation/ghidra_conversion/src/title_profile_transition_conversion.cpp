#include "shadowrun_ghidra/title_profile_transition_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

bool decrement_word_until_negative(std::uint16_t& value) {
    value = static_cast<std::uint16_t>(value - 1U);
    return (value & 0x8000U) != 0U;
}

} // namespace

TitleProfileTransitionAction advance_title_profile_transition(
    TitleProfileTransitionState& state,
    const TitleProfileTransitionCallbacks& callbacks) {
    state.dirty_flags |= 1U;
    if (decrement_word_until_negative(state.countdown)) {
        if (callbacks.copy_title_profile) {
            callbacks.copy_title_profile();
        }
        state.behavior_flags = static_cast<std::uint8_t>(
            state.behavior_flags & ~0x80U);
        if (callbacks.attach_behavior_descriptor_b) {
            callbacks.attach_behavior_descriptor_b();
        }
        if (callbacks.run_menu_slot_service) {
            callbacks.run_menu_slot_service();
        }
        if (callbacks.initialize_world_effect_mode) {
            callbacks.initialize_world_effect_mode();
        }
        return TitleProfileTransitionAction::finalized;
    }

    const auto phase = static_cast<std::uint16_t>(
        ((0U - state.countdown) >> 1U) & 0x0003U);
    const auto token = callbacks.resolve_sprite_data
                           ? callbacks.resolve_sprite_data(0x0001C136U, phase)
                           : std::nullopt;
    if (!token.has_value()) {
        return TitleProfileTransitionAction::sprite_data_unresolved;
    }
    if (callbacks.build_sprite) {
        callbacks.build_sprite(TitleProfileTransitionSpriteBuild{
            0x0001C136U, phase, 0x015EU, 0x00EEU, 0x2000U, *token});
        callbacks.build_sprite(TitleProfileTransitionSpriteBuild{
            0x0001C136U, phase, 0x00E0U, 0x00EEU, 0x2000U, *token});
    }
    return TitleProfileTransitionAction::rendered;
}

ProfileCopyTimerAction advance_profile_copy_timer(
    ProfileCopyTimerState& state,
    const ProfileCopyTimerCallbacks& callbacks) {
    state.countdown = static_cast<std::int16_t>(state.countdown - 1);
    if (state.countdown >= 0) {
        return ProfileCopyTimerAction::waiting;
    }

    const auto offset_words = static_cast<std::int16_t>(state.countdown + 8);
    if (offset_words < 0 && callbacks.resolve_reload_countdown) {
        const auto reload = callbacks.resolve_reload_countdown(
            state.current_record_timing_value);
        if (!reload.has_value()) {
            return ProfileCopyTimerAction::reload_unresolved;
        }
        state.countdown = *reload;
        return ProfileCopyTimerAction::reloaded;
    }

    if (offset_words < 0) {
        return ProfileCopyTimerAction::reload_unresolved;
    }

    if (!callbacks.read_copy_eligibility) {
        return ProfileCopyTimerAction::eligibility_unresolved;
    }
    const auto eligibility = callbacks.read_copy_eligibility();
    if (!eligibility.has_value()) {
        return ProfileCopyTimerAction::eligibility_unresolved;
    }
    if (*eligibility == 0U) {
        return ProfileCopyTimerAction::copy_ineligible;
    }
    if (callbacks.copy_profile_words) {
        callbacks.copy_profile_words(
            static_cast<std::int32_t>(offset_words) * 2);
    }
    if (callbacks.mark_profile_dirty) {
        callbacks.mark_profile_dirty();
    }
    return ProfileCopyTimerAction::copied;
}

} // namespace shadowrun::ghidra_conversion
