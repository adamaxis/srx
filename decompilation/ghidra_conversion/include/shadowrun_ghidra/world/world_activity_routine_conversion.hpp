#pragma once

#include "shadowrun_ghidra/field/scene_profile_conversion.hpp"
#include "shadowrun_ghidra/world/world_activity_mode.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

enum class WorldActivityResult {
    field_ready,
    existing_activity_still_active,
    blocked_by_scene_gate,
    contact_flags_unavailable,
    timeout_scheduled,
    resource_activity_ready,
    resource_activity_bound_unavailable,
    resource_activity_random_input_unavailable,
    resource_activity_sample_out_of_range,
    scripted_activity_ready,
    world_script_selected,
    halted,
};

struct WorldActivityResourceOverrides {
    bool force_index_five_marker_zero = false; // FFE184
    bool force_index_two_marker_three = false; // FFE187
    bool force_index_four = false; // FFE180
};

struct WorldActivityRoutineInput {
    std::uint16_t initial_scene_id = 0;
    // A Lua location route may provide the typed branch selected from the
    // ROM event-table value. Source-internal transitions leave this empty and
    // use the ROM-derived scene-range classifier below.
    std::optional<WorldActivityMode> routed_destination_mode;
    // ROM FFFBF9 is the selected field-event/mode-table index consumed by
    // 0x167AE. Scene 10 uses value 6 to select its alternate runner/list row.
    std::uint8_t mode_event_code = 0;
    std::uint8_t timeout_override = 0;
    bool descriptors_pending = false;
    bool existing_activity = false;
    std::uint32_t world_flags = 0; // FFFC1E story flags
    std::optional<std::uint32_t> contact_flags; // FFFC2C contacts and scene flags
    std::uint8_t primary_runner_marker = 0xFFU;
    std::uint8_t secondary_runner_marker = 0xFFU;
    std::vector<std::int8_t> runner_slot_by_scene;
    std::vector<bool> world_script_available;
    std::uint16_t default_resource_word = 0; // FFF0C8 on resource entry
    SceneResourceProfileFields resource_cursor_source{};

    // ROM 0x167AE's resource branch reads the bound from the selected profile
    // (FFF0C6), samples an index into FFE12C, then advances the PRNG again and
    // stores its low three bits as the marker ordinal in FFE185.
    std::optional<std::uint16_t> resource_random_index_bound;
    WorldActivityResourceOverrides resource_overrides{};
};

struct WorldActivityRoutineState {
    std::uint16_t scene_id = 0;
    WorldActivityMode mode = WorldActivityMode::field;
    std::uint32_t world_flags = 0;
    std::optional<std::uint32_t> contact_flags;
    std::uint16_t resource_random_index_bound = 0; // FFF0C6
    std::uint16_t selected_resource_random_index = 0; // FFE12C
    std::uint8_t resource_marker_ordinal = 0; // FFE185
    std::uint16_t default_resource_word = 0; // initial FFF0C8 value
    SceneResourceCursorState resource_cursor{};
    std::optional<std::size_t> selected_world_script;
    bool selected_scene_ten_variant_script = false;
    bool scene_snapshot_taken = false;
    bool field_scene_prepared = false;
    bool timeout_requested = false;
    bool descriptors_published = false;
    bool mode_word_upload_requested = false;
    bool world_activity_effect_words_cleared = false;
};

struct WorldActivityRoutineCallbacks {
    std::function<void()> schedule_world_activity_timeout;
    std::function<void()> publish_world_activity_descriptors;
    std::function<void()> snapshot_scene_state;
    std::function<void()> prepare_field_scene_buffers;
    std::function<void()> upload_mode_word_block;
    std::function<std::optional<std::uint16_t>(std::uint16_t)>
        sample_source_random_bound;
    std::function<std::optional<std::uint16_t>()> advance_source_prng;
};

// ROM 0x167AE: read the scene/mode byte selected by FFFBF9 from the active
// 32-byte table published through FFDE14. The host receives the extracted
// table directly; event codes outside that source table are rejected instead
// of becoming unchecked pointer arithmetic.
std::optional<std::uint16_t> resolve_world_activity_scene_id(
    std::uint8_t mode_event_code,
    const std::array<std::uint8_t, 32>& event_mode_table);

// Populate the one-time typed import of the selector tables at 0xEEFE and
// 0x16FD4. These are host values; no ROM address arithmetic is performed at
// runtime.
void populate_source_world_activity_route_tables(
    WorldActivityRoutineInput& input);

// ROM 0x167AE: choose the current scene/activity branch, perform the source
// flag gates, select a runner-dependent world script, and prepare the
// 0x15..0x37 resource/activity modes. ROM tables are supplied as host vectors;
// no source pointer or Genesis RAM address crosses this boundary.
WorldActivityResult run_world_activity_routine(
    const WorldActivityRoutineInput& input,
    WorldActivityRoutineState& state,
    const WorldActivityRoutineCallbacks& callbacks = {});

enum class WorldActivityCallbackKind {
    none,
    extraction_result, // ROM 0x1E49C installs callback 0x1F16A
};

struct WorldActivityStartState {
    std::uint16_t timer_frames = 0; // FFE11C
    WorldActivityCallbackKind callback = WorldActivityCallbackKind::none;
};

// ROM 0x1E2EC starts FFE11C and publishes the already-resolved callback as
// FFE104. The host stores its typed continuation rather than a code pointer.
void begin_world_activity(
    WorldActivityStartState& state,
    WorldActivityCallbackKind callback);

struct WorldActivityTimeoutState {
    std::uint16_t scene_id = 0; // FFDDC4
    std::uint8_t encounter_actor_count = 0; // FFE177 [C]
    std::int16_t timeout_marker = 0; // FFDF84
    std::uint16_t timeout_frames = 0; // FFDF86
    std::uint32_t extended_timeout = 0; // FFE162
};

// ROM 0x1DBAA: scene 0x36 with FFE177==0 RTS. Else if FFDF84==0 write -1,
// set FFDF86 to 0x78, or 0x258 when FFE162!=0.
bool schedule_world_activity_timeout(WorldActivityTimeoutState& state);

struct WorldActivityRequestFacts {
    bool selection_made = false;
    bool active_escort_present = false;
};

struct WorldActivityRequestOutcome {
    bool active_escort_present = false;
};

// [C] A selected Employee Recruitment contact starts the extraction-result
// flow. The presence of a live escort affects only which result is shown.
// ROM 0x1E43C is the source for these semantics; its record layout stays in
// the native field-runtime adapter rather than this gameplay model.
std::optional<WorldActivityRequestOutcome> handle_world_activity_request(
    const WorldActivityRequestFacts& facts,
    WorldActivityStartState& activity);

} // namespace shadowrun::ghidra_conversion
