#pragma once

#include <array>
#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion {

// ROM 0x03032: identify the scene IDs that use the alternate field profile.
// The source accepts 6..9 and 0x2C..0x31, inclusive.
bool scene_uses_alternate_profile(std::int16_t scene_id);

struct SceneRunnerPosition {
    std::uint16_t x = 0;
    std::uint16_t y = 0;
};

struct SceneProfileDefaults {
    std::uint16_t camera_x = 0;
    std::uint16_t camera_y = 0;
    std::array<SceneRunnerPosition, 3> runner_positions{};
};

struct SceneProfileState {
    std::uint16_t camera_x = 0;
    std::uint16_t camera_y = 0;
    std::array<SceneRunnerPosition, 3> runner_positions{};
    std::int16_t camera_transition_x = 0;
    std::int16_t camera_transition_y = 0;
};

// ROM 0x2B34: apply either the six default profile words or one pending
// eight-word profile. Pending camera coordinates are aligned to 0x80 and
// reset both source transition markers to -1.
void apply_scene_profile_selection(
    SceneProfileState& state,
    std::optional<std::array<std::uint16_t, 8>>& pending_profile,
    const SceneProfileDefaults& defaults);

} // namespace shadowrun::ghidra_conversion
