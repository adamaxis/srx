#pragma once

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct TitleProfileTransitionState {
    std::uint16_t profile_selector = 0; // FFDF88
    std::uint16_t countdown = 0;         // FFE00C
    std::uint8_t behavior_flags = 0;    // FF1592
    std::uint32_t dirty_flags = 0;      // FFEF14
};

struct TitleProfileTransitionSpriteBuild {
    std::uint32_t source_table_token = 0x0001C136U;
    std::uint16_t phase = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0x00EEU;
    std::uint16_t build_flags = 0x2000U;
    std::uint32_t sprite_data_token = 0;
};

struct TitleProfileTransitionCallbacks {
    // ROM 0x1BF5C copies the selected seven-word title profile.
    std::function<void()> copy_title_profile;
    std::function<void()> attach_behavior_descriptor_b;
    std::function<void()> run_menu_slot_service;
    std::function<void()> initialize_world_effect_mode;
    // Resolve ROM 0x1C136's four-entry sprite table and its +8 payload.
    std::function<std::optional<std::uint32_t>(
        std::uint32_t source_table_token, std::uint16_t phase)>
        resolve_sprite_data;
    std::function<void(const TitleProfileTransitionSpriteBuild&)> build_sprite;
};

enum class TitleProfileTransitionAction {
    rendered,
    finalized,
    sprite_data_unresolved,
};

// ROM 0x1BEBA: copy the selected profile words into the source destination,
// advance the transition countdown, render the two profile sprites, or run
// the source finalization sequence when the countdown expires.
TitleProfileTransitionAction advance_title_profile_transition(
    TitleProfileTransitionState& state,
    const TitleProfileTransitionCallbacks& callbacks);

struct ProfileCopyTimerState {
    std::int16_t countdown = 0; // FFF11E
    std::int8_t current_record_timing_value = 0; // current record +0x06
};

struct ProfileCopyTimerCallbacks {
    // ROM 0x1BFFC is indexed by the signed current-record byte.
    std::function<std::optional<std::int16_t>(std::int8_t timing_value)>
        resolve_reload_countdown;
    // ROM 0x17C3C returns the source profile-copy eligibility value.
    std::function<std::optional<std::uint16_t>()> read_copy_eligibility;
    // ROM 0x1C012 copies seven words from 0x1C022 plus the signed byte offset.
    std::function<void(std::int32_t signed_byte_offset)> copy_profile_words;
    std::function<void()> mark_profile_dirty;
};

enum class ProfileCopyTimerAction {
    waiting,
    reloaded,
    copied,
    copy_ineligible,
    eligibility_unresolved,
    reload_unresolved,
};

// ROM 0x1BF7A: decrement the signed profile timer, reload it from the
// current compact record once it passes the -8 threshold, or copy the seven
// words selected by the negative timer when the source gate is positive.
ProfileCopyTimerAction advance_profile_copy_timer(
    ProfileCopyTimerState& state,
    const ProfileCopyTimerCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
