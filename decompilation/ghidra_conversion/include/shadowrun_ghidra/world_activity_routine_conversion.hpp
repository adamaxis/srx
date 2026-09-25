#pragma once

#include "shadowrun_ghidra/scene_profile_conversion.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

enum class WorldActivityMode {
    field = 1,
    resource_activity = 2,
    scripted_activity = 3,
    halted = 4,
};

enum class WorldActivityResult {
    field_ready,
    existing_activity_still_active,
    blocked_by_scene_gate,
    timeout_scheduled,
    resource_activity_ready,
    scripted_activity_ready,
    world_script_selected,
    halted,
};

struct WorldActivityRoutineInput {
    std::uint16_t initial_scene_id = 0;
    std::uint8_t scene_variant = 0;
    std::uint8_t timeout_override = 0;
    bool descriptors_pending = false;
    bool existing_activity = false;
    std::uint32_t world_flags = 0;
    std::uint32_t mode_flags = 0;
    std::uint8_t primary_runner_marker = 0xFFU;
    std::uint8_t secondary_runner_marker = 0xFFU;
    std::vector<std::int8_t> runner_slot_by_scene;
    std::vector<bool> world_script_available;
    std::uint16_t default_resource_word = 0;
    SceneResourceProfileFields resource_cursor_source{};

    // These are the source values used by the encounter/resource branch.
    std::uint16_t sampled_random_bound = 0;
    std::uint8_t advanced_prng_value = 0;
    bool force_random_pair_five_zero = false;
    bool force_random_pair_two_three = false;
    bool force_random_bound_four = false;
};

struct WorldActivityRoutineState {
    std::uint16_t scene_id = 0;
    WorldActivityMode mode = WorldActivityMode::field;
    std::uint32_t world_flags = 0;
    std::uint16_t random_bound = 0;
    std::uint8_t random_slot = 0;
    std::uint16_t default_resource_word = 0;
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
};

// ROM 0x167AE: choose the current scene/activity branch, perform the source
// flag gates, select a runner-dependent world script, and prepare the
// 0x15..0x37 resource/activity modes. ROM tables are supplied as host vectors;
// no source pointer or Genesis RAM address crosses this boundary.
WorldActivityResult run_world_activity_routine(
    const WorldActivityRoutineInput& input,
    WorldActivityRoutineState& state,
    const WorldActivityRoutineCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
