#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>

#include "shadowrun_ghidra/field/field_mode_profile.hpp"

namespace shadowrun::ghidra_conversion {

struct FieldModeEntryTransientState {
    // ROM FFE12E: source clears the complete deferred activity handler word.
    std::uint32_t pending_world_activity_handler = 0;
    // ROM FFE180/FFE182: active and pending world-effect markers.
    std::uint8_t world_effect_active = 0;
    std::uint8_t pending_world_effect = 0;
    // ROM FFE184/FFE187: effect-context selectors used by terrain/effect work.
    std::uint8_t effect_context_active = 0;
    std::uint8_t alternate_effect_context = 0;
    // ROM FFE18F/FFE183: mode-five marker and world-activity request.
    std::uint8_t mode_five_effect_marker = 0;
    std::uint8_t world_activity_request = 0;
};

struct FieldModeInitializationState {
    FieldModeProfile profile = FieldModeProfile::profile_a;
    FieldModeEntryTransientState entry_transient{};
    // ROM FFDF84: world/effect activity gate cleared by both profile entries.
    bool world_effects_active = false;
};

struct FieldModeInitializationCallbacks {
    std::function<void(FieldModeProfile)> select_profile_data;
    std::function<void()> prepare_mode_entry;
    std::function<void()> initialize_scene_mode_state;
};

// ROM 0x2A32 / 0x2A44: install one of the two field-mode profile records, run
// the common mode-entry service, clear the typed FFE12E/FFE18x transient
// fields and the FFDF84 world/effect activity gate, then initialize
// scene-mode state.
// Profile tables stay a host-owned data boundary rather than becoming
// ROM-addressed runtime state.
void initialize_field_mode(
    FieldModeInitializationState& state,
    FieldModeProfile profile,
    const FieldModeInitializationCallbacks& callbacks);

// ROM 0x2E72/0x2E42/0x2ED2/0x2D92 room graph tables live only in Lua
// (world_links.cave_exit_flat, data/world_links_data.lua room_graph). The
// host resolves FFFBF9/FFFC10 there and passes the exit in.
struct FieldModeTransitionExit {
    std::int16_t next_mode_index = 0; // FFFC10 (0x2E72 byte, EXT.W)
    std::uint16_t scene_id = 0; // FFDDC4 (0x2E42)
    std::uint16_t override_x = 0; // FFE10A (0x2D92 pair)
    std::uint16_t override_y = 0; // FFE10C
};

enum class FieldModePendingActorList {
    council_island_spirit_eyes,
};

std::string_view field_mode_pending_actor_list_id(
    FieldModePendingActorList actor_list) noexcept;

struct FieldModeTransitionInput {
    std::uint8_t event_code = 0; // FFFBF9
    std::int16_t transition_context = 0; // FFFC10
    std::uint32_t world_activity_flags = 0; // FFFC1E long, BTST #8
    // Lua-resolved room graph exit; nullopt when the lookup is out of table.
    std::optional<FieldModeTransitionExit> exit;
};

struct FieldModeTransitionPlan {
    std::int16_t next_mode_index = 0; // FFFC10
    std::uint16_t scene_id = 0; // FFDDC4
    std::uint16_t override_x = 0; // FFE10A
    std::uint16_t override_y = 0; // FFE10C
    bool resolved = false;
    bool spawn_position_available = false;
    bool start_world_activity = false;
    std::optional<FieldModePendingActorList> pending_actor_list;
};

struct FieldModeTransitionCallbacks {
    std::function<void()> begin_world_activity; // 0x1E2EC
    std::function<void()> schedule_timeout; // 0x1DBAA
};

// ROM 0x2CE0: publish the Lua-resolved 0x2E72/0x2E42/0x2ED2/0x2D92 exit. Mode
// 0x13 with FFFC1E bit 8 clear publishes 0x53EE2/0x1E552 and JSRs 0x1E2EC
// then 0x1DBAA. D0/D1 for the select lookup stay the pre-JSR values.
FieldModeTransitionPlan advance_field_mode_transition(
    const FieldModeTransitionInput& input,
    const FieldModeTransitionCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
