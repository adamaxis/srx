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

// ROM 0x2F32 DBF from D7=#0x30 copies 49 words FFDE26 → FFDDC4, then
// overwrites words 1-2 from FFDF50/FFDF52. Falls into 0x2FA4.
constexpr std::size_t kFieldSceneRuntimeStateWordCount = 49;

struct FieldSceneRuntimeState {
    std::uint16_t scene_gate = 0; // FFDF40, always cleared
    std::array<std::uint16_t, kFieldSceneRuntimeStateWordCount> profile{};
    std::uint8_t scene_byte = 0; // FFFBF8
    std::uint16_t alternate_profile = 0; // FFFC0E from 0x3032
    std::int16_t fade_marker = 0; // FFDF8A
};

struct FieldSceneRuntimeStateCallbacks {
    std::function<void()> prepare_field_scene_runtime; // 0xC7F8
    std::function<void()> advance_field_scene_transition; // 0x2BD2
    std::function<void()> upload_scene_profile_block; // 0x301E
};

// ROM 0x2F32: JSR 0xC7F8, CLR FFDF40, FFDF8A=-1, copy 49 backup words,
// MOVE.B scene into FFFBF8, BSR 0x3032, write FFDF50/52 into FFDDC6/C8,
// BSR 0x2BD2 then 0x301E. FFDF40==2 samples FFFC22 bit 10 with no effect
// on 0xC7F8 (TST.B FF014C overwrites CCR).
void initialize_field_scene_state(
    FieldSceneRuntimeState& state,
    const std::array<std::uint16_t, kFieldSceneRuntimeStateWordCount>& backup,
    std::uint16_t saved_camera_x,
    std::uint16_t saved_camera_y,
    const FieldSceneRuntimeStateCallbacks& callbacks);

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

// ROM 0x31F2/0x3202/0x3212/0x3222: eight-word pending profiles published by
// 0x3168 into FFE114 and consumed by 0x2B34 / 0x2FA4.
constexpr std::array<std::uint16_t, 8> kFieldSceneVariantProfile0x31F2{{
    0x6680U, 0x2500U, 0x7400U, 0x2C00U, 0x7380U, 0x2C80U, 0x7480U, 0x2C80U}};
constexpr std::array<std::uint16_t, 8> kFieldSceneVariantProfile0x3202{{
    0x0000U, 0x2380U, 0x0300U, 0x2A80U, 0x0300U, 0x2B00U, 0x0300U, 0x2B80U}};
constexpr std::array<std::uint16_t, 8> kFieldSceneVariantProfile0x3212{{
    0x0000U, 0x3200U, 0x0600U, 0x3A00U, 0x0600U, 0x3A80U, 0x0600U, 0x3B00U}};
constexpr std::array<std::uint16_t, 8> kFieldSceneVariantProfile0x3222{{
    0x6CF0U, 0x2400U, 0x7880U, 0x2B00U, 0x7880U, 0x2B00U, 0x7880U, 0x2B00U}};

struct FieldSceneVariantState {
    std::uint8_t field_mode = 0; // FFFBF9
    std::uint16_t scene_id = 0; // FFDDC4
    std::uint16_t camera_x = 0; // FFDDC6
    std::optional<std::array<std::uint16_t, 8>> pending_profile;
};

// ROM 0x3168: mode 0x10 publishes one pending profile and sets CCR carry.
// Other modes ANDI CCR #0x1E (clear carry) and leave scene/profile alone.
bool toggle_field_scene_variant(FieldSceneVariantState& state);

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
