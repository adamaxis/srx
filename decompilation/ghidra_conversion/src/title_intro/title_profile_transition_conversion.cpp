#include "shadowrun_ghidra/title_intro/title_profile_transition_conversion.hpp"

#include <array>

namespace shadowrun::ghidra_conversion {
namespace {

constexpr std::array<std::array<std::uint16_t, 7>, 8>
    kTitleProfileWordWindows{{
        {{0x0000U, 0x0222U, 0x0444U, 0x0666U, 0x0888U, 0x0AAAU,
          0x0EEEU}},
        {{0x0222U, 0x0444U, 0x0666U, 0x0888U, 0x0AAAU, 0x0EEEU,
          0x0000U}},
        {{0x0444U, 0x0666U, 0x0888U, 0x0AAAU, 0x0EEEU, 0x0000U,
          0x0222U}},
        {{0x0666U, 0x0888U, 0x0AAAU, 0x0EEEU, 0x0000U, 0x0222U,
          0x0444U}},
        {{0x0888U, 0x0AAAU, 0x0EEEU, 0x0000U, 0x0222U, 0x0444U,
          0x0666U}},
        {{0x0AAAU, 0x0EEEU, 0x0000U, 0x0222U, 0x0444U, 0x0666U,
          0x0888U}},
        {{0x0EEEU, 0x0000U, 0x0222U, 0x0444U, 0x0666U, 0x0888U,
          0x0AAAU}},
        {{0x0000U, 0x0222U, 0x0444U, 0x0666U, 0x0888U, 0x0AAAU,
          0x0EEEU}},
    }};
constexpr std::array<std::uint32_t, 4> kTitleProfileTransitionSpriteData{
    0x0001D8C6U, 0x0001D8D0U, 0x0001D8DAU, 0x0001D8E4U};
constexpr std::array<std::int16_t, 11> kTitleProfileReloadCountdowns{
    0x00F0, 0x00D2, 0x00B4, 0x0078, 0x005A, 0x002D,
    0x001E, 0x0014, 0x000A, 0x0008, 0x0005};

bool decrement_word_until_negative(std::uint16_t& value) {
    value = static_cast<std::uint16_t>(value - 1U);
    return (value & 0x8000U) != 0U;
}

} // namespace

std::optional<std::array<std::uint16_t, 7>> resolve_title_profile_words(
    const std::uint16_t profile_selector) {
    if (profile_selector >= kTitleProfileWordWindows.size()) {
        return std::nullopt;
    }
    return kTitleProfileWordWindows[profile_selector];
}

std::optional<std::uint32_t> resolve_title_profile_transition_sprite_data(
    const std::uint16_t phase) {
    if (phase >= kTitleProfileTransitionSpriteData.size()) {
        return std::nullopt;
    }
    return kTitleProfileTransitionSpriteData[phase];
}

std::optional<std::int16_t> resolve_title_profile_reload_countdown(
    const std::int8_t timing_value) {
    if (timing_value < 0 ||
        static_cast<std::size_t>(timing_value) >=
            kTitleProfileReloadCountdowns.size()) {
        return std::nullopt;
    }
    return kTitleProfileReloadCountdowns[static_cast<std::size_t>(
        timing_value)];
}

TitleProfileTransitionAction advance_title_profile_transition(
    TitleProfileTransitionState& state,
    const TitleProfileTransitionCallbacks& callbacks) {
    state.dirty_flags |= 1U;
    const auto profile_selector = static_cast<std::uint16_t>(
        state.profile_selector & 0x0007U);
    const auto profile_words = callbacks.resolve_profile_words
                                   ? callbacks.resolve_profile_words(
                                         profile_selector)
                                   : resolve_title_profile_words(
                                         profile_selector);
    if (!profile_words.has_value()) {
        return TitleProfileTransitionAction::profile_data_unresolved;
    }
    state.profile_destination_words = *profile_words;
    if (callbacks.publish_profile_destination) {
        callbacks.publish_profile_destination(*profile_words);
    }
    if (decrement_word_until_negative(state.countdown)) {
        const auto fixed_profile = resolve_title_profile_words(0U);
        if (fixed_profile.has_value()) {
            state.profile_destination_words = *fixed_profile;
            if (callbacks.publish_profile_destination) {
                callbacks.publish_profile_destination(*fixed_profile);
            }
        }
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
                           : resolve_title_profile_transition_sprite_data(phase);
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
    if (offset_words < 0) {
        const auto reload = callbacks.resolve_reload_countdown
                                ? callbacks.resolve_reload_countdown(
                                      state.current_record_timing_value)
                                : resolve_title_profile_reload_countdown(
                                      state.current_record_timing_value);
        if (!reload.has_value()) {
            return ProfileCopyTimerAction::reload_unresolved;
        }
        state.countdown = *reload;
        return ProfileCopyTimerAction::reloaded;
    }

    if (!callbacks.read_copy_eligibility) {
        return ProfileCopyTimerAction::eligibility_unresolved;
    }
    const auto eligibility = callbacks.read_copy_eligibility();
    if (!eligibility.has_value()) {
        return ProfileCopyTimerAction::eligibility_unresolved;
    }
    if (*eligibility <= 0) {
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
