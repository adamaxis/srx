#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct TitleProfileTransitionState {
    std::uint16_t profile_selector = 0; // FFDF88
    std::uint16_t countdown = 0;         // FFE00C
    std::uint8_t behavior_flags = 0;    // FF1592
    std::uint32_t dirty_flags = 0;      // FFEF14
    std::array<std::uint16_t, 7> profile_destination_words{}; // FFD880
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
    // ROM 0x1BEBA copies a seven-word window selected by FFDF88 & 7 from
    // 0x1C10A before it advances the transition countdown.
    std::function<std::optional<std::array<std::uint16_t, 7>>(
        std::uint16_t profile_selector)>
        resolve_profile_words;
    std::function<void(const std::array<std::uint16_t, 7>&)>
        publish_profile_destination;
    // ROM 0x1BF5C copies the fixed seven-word profile at 0x1C10A when the
    // transition expires.
    std::function<void()> copy_title_profile;
    std::function<void()> attach_behavior_descriptor_b;
    std::function<void()> run_menu_slot_service;
    std::function<void()> initialize_world_effect_mode;
    // Resolve ROM 0x1C136's four-entry table. Its entries are direct inline
    // sprite-record streams, not pointers requiring a synthetic +8 offset.
    std::function<std::optional<std::uint32_t>(
        std::uint32_t source_table_token, std::uint16_t phase)>
        resolve_sprite_data;
    std::function<void(const TitleProfileTransitionSpriteBuild&)> build_sprite;
};

enum class TitleProfileTransitionAction {
    profile_data_unresolved,
    rendered,
    finalized,
    sprite_data_unresolved,
};

// Typed extraction of the eight seven-word windows selected by FFDF88 & 7
// from ROM 0x1C10A.
std::optional<std::array<std::uint16_t, 7>>
resolve_title_profile_words(std::uint16_t profile_selector);

// ROM 0x1C136 contains four direct inline sprite-record streams.
std::optional<std::uint32_t>
resolve_title_profile_transition_sprite_data(std::uint16_t phase);

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
    // ROM 0x17C3C returns the indexed FFFB30 profile-copy eligibility value
    // as a signed result; 0x1BF7A uses BLE, so zero and negative results
    // suppress the copy.
    std::function<std::optional<std::int16_t>()> read_copy_eligibility;
    // ROM 0x1C012 copies seven words from 0x1C022 plus the signed byte offset.
    std::function<void(std::int32_t signed_byte_offset)> copy_profile_words;
    std::function<void()> mark_profile_dirty;
};

// ROM 0x1BFFC contains the reload countdown values indexed by the signed
// current-record timing byte. Negative indexes remain an unresolved source
// boundary rather than being wrapped into an unrelated table.
std::optional<std::int16_t> resolve_title_profile_reload_countdown(
    std::int8_t timing_value);

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
