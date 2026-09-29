#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

// ROM 0x03032: identify the scene IDs that use the alternate field profile.
// The source accepts 6..9 and 0x2C..0x31, inclusive.
bool scene_uses_alternate_profile(std::int16_t scene_id);

constexpr std::size_t kFieldSceneProfileWordCount = 48;
constexpr std::size_t kFieldSceneProfileCount = 56;

// ROM 0x3232: one long per scene id 0..0x37.
constexpr std::array<std::uint32_t, kFieldSceneProfileCount>
    kFieldSceneProfileTable{{
        0x000D5456U, 0x0017D5C4U, 0x00172C80U, 0x0018A6A8U, 0x0015B902U,
        0x0006796AU, 0x0014B890U, 0x001B3D0EU, 0x001B45BCU, 0x001B4E1AU,
        0x001D609AU, 0x001D5A44U, 0x001D5F8CU, 0x001B3C06U, 0x001D5B52U,
        0x001D5C4CU, 0x001D61C8U, 0x001D62CCU, 0x001D5D7AU, 0x001D5E74U,
        0x001D917AU, 0x001D73E4U, 0x001D7A86U, 0x001D81BAU, 0x001D7A86U,
        0x001D8A40U, 0x001D97ECU, 0x001D9ED2U, 0x001DA6A2U, 0x001DAE2CU,
        0x001DAEB8U, 0x001DB49CU, 0x001DBA94U, 0x001DC130U, 0x001DC1BCU,
        0x001DC248U, 0x001DC9A8U, 0x001DC9A8U, 0x001DD100U, 0x001DD866U,
        0x001DDD1AU, 0x001DE234U, 0x001DE83EU, 0x001DEE62U, 0x001D48AAU,
        0x001D4BD0U, 0x001D4E9AU, 0x001D51A0U, 0x001D54F6U, 0x001D57DEU,
        0x001E8D3CU, 0x001E911CU, 0x001E94DCU, 0x001E96E4U, 0x001E98ECU,
        0x001EDA28U}};

struct FieldSceneLoopState {
    std::uint16_t scene_id = 0;
    std::uint16_t alternate_profile = 0; // FFFC0E
    std::uint16_t override_lock = 0; // FFE10E
    std::uint16_t override_x = 0; // FFE10A
    std::uint16_t override_y = 0; // FFE10C
    std::array<std::uint16_t, kFieldSceneProfileWordCount> profile{};
};

std::uint32_t field_scene_profile_table_entry(std::uint16_t scene_id);

// ROM 0x3084: copy 48 profile words, MOVE.L FFDDDE to FFDE22, then when
// FFFC0E is set and FFE10E is clear write FFE10A/C into words 33-38 and
// camera words 0-1 (subtract 0xA00/0x700, clamp 0). Always CLR FFE10E.
void run_field_loop(
    FieldSceneLoopState& state,
    const std::array<std::uint16_t, kFieldSceneProfileWordCount>& source);

using FieldTerrainLayerCopy = std::function<void(
    std::uint32_t source, std::uint32_t destination, std::size_t words)>;

// ROM 0x313A: 0xE10 words *FFDDDA -> $FF2700, then *FFDDE2 -> $FF4700.
std::size_t copy_terrain_layers(
    std::uint32_t source_a,
    std::uint32_t source_b,
    const FieldTerrainLayerCopy& copy_block);

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

struct SceneResourceCursorState {
    std::int8_t cursor = 0;
    std::uint16_t resource_index = 0;
};

struct SceneResourceProfileFields {
    std::int8_t cursor = 0;
    std::uint16_t resource_index = 0;
};

// ROM 0x16B7C: publish the two scene-profile words used by the menu/HUD
// resource cursor. The original words remain a source-layout detail.
void copy_scene_resource_cursor(
    SceneResourceCursorState& state,
    const SceneResourceProfileFields& source);

struct SceneRecordPlacement {
    std::int16_t x = 0;
    std::int16_t y = 0;
    std::int16_t kind = 0;
    std::uint8_t flags = 0;
};

// ROM 0x16D8A: after the shared slot-availability check, create the offset
// scene record and mark it active. Returns false when all destination slots
// are occupied.
bool spawn_scene_record_offset(
    SceneRecordPlacement& destination,
    const SceneRecordPlacement& source,
    std::int16_t kind,
    const std::function<bool()>& all_slots_active);

} // namespace shadowrun::ghidra_conversion
