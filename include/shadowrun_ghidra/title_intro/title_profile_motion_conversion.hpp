#pragma once

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct TitleProfileMotionState {
    std::uint16_t motion_mode = 0;       // FFE058
    std::int8_t profile_frame_delay = 0; // FF1580 +0x14
    std::uint8_t profile_phase = 0;      // FF1580 +0x13
    std::uint16_t profile_x = 0;         // FF1580 +0x0E
    std::uint16_t profile_y = 0;         // FF1580 +0x10
    std::uint16_t movement_countdown = 0; // FFE052
    std::uint16_t moving_x = 0;          // FFE056
    std::uint16_t moving_y = 0;          // FFE054
    std::uint16_t secondary_frame = 0;   // FFE05C
    std::uint16_t secondary_phase = 0;   // FFE05A
    std::uint16_t transition_state = 0; // FFE0D6
    std::uint16_t completion_state = 0; // FFE0F4
    bool pending_selection = false;     // FFE0C8
};

struct TitleProfileMotionSprite {
    std::uint32_t source_table_token = 0;
    std::int16_t offset_x = 0;
    std::int16_t offset_y = 0;
};

struct TitleProfileMotionSpriteBuild {
    std::uint32_t source_table_token = 0;
    std::uint16_t phase = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t build_flags = 0x6000U;
    std::uint32_t sprite_data_token = 0;
    bool moving_sprite = false;
};

struct TitleProfileMotionCallbacks {
    // ROM 0x1BE4E selects one of the four inline profile sprite streams at
    // 0x1BE3E and supplies signed x/y offsets for the 13 profile phases.
    std::function<std::optional<TitleProfileMotionSprite>(
        std::uint16_t phase)>
        resolve_profile_sprite;
    // ROM 0x1BE3E supplies the four inline moving-frame sprite streams.
    std::function<std::optional<std::uint32_t>(
        std::uint16_t phase)>
        resolve_moving_sprite;
    // ROM 0x15E56 provides one easing word for each elapsed movement step.
    std::function<std::optional<std::int16_t>(std::uint16_t elapsed_step)>
        resolve_movement_easing;
    std::function<void(const TitleProfileMotionSpriteBuild&)> build_sprite;
    std::function<void()> refresh_menu_selection_or_prepare;
};

// Typed extraction of the 13 profile-phase records at ROM 0x1BE4E and the
// four inline sprite streams selected through ROM 0x1BE3E.
std::optional<TitleProfileMotionSprite> resolve_title_profile_phase_sprite(
    std::uint16_t phase);
std::optional<std::uint32_t> resolve_title_profile_moving_sprite_data(
    std::uint16_t phase);

// ROM 0x15E56 is indexed by ((elapsed_step * 3) & 0xFE), not by a packed
// host array stride. The extracted host table preserves those 60 source
// samples for the movement easing.
std::optional<std::int16_t> resolve_title_profile_movement_easing(
    std::uint16_t elapsed_step);

enum class TitleProfileMotionAction {
    updated,
    rendered,
    finalized,
    sprite_data_unresolved,
    easing_unresolved,
};

// ROM 0x1BCE8: advance the profile's 13-frame sprite phase, then its
// 60-step movement/easing and four-frame secondary sprite cycle. The source
// finalization path clears the motion/menu state and publishes completion.
TitleProfileMotionAction advance_title_profile_motion(
    TitleProfileMotionState& state,
    const TitleProfileMotionCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
