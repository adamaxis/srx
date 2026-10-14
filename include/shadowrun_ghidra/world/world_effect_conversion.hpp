#pragma once

#include "shadowrun_ghidra/world/world_activity_routine_conversion.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

namespace shadowrun::ghidra_conversion {

// ROM 0x1DF00 / 0x1E0B8: D7=#6 walks seven FF0400 records at +0x9E.
constexpr std::size_t kWorldEffectFlagSlotCount = 7;

// ROM 0x1DF00: BCLR #0 of +0x9E on each of the seven slots.
void clear_entity_effect_pending_flags(
    std::array<std::uint8_t, kWorldEffectFlagSlotCount>& effect_flags);

// ROM 0x1E0B8: BTST #1 of +0x9E then BSET #2, seven slots.
void promote_entity_effect_flags(
    std::array<std::uint8_t, kWorldEffectFlagSlotCount>& effect_flags);

enum class WorldEffectTransitionAction {
    no_action,
    timeout_scheduled,
    already_latched,
    started,
};

struct WorldEffectTransitionState {
    std::uint16_t scene_id = 0;
    std::uint16_t world_mode = 0;
    std::int16_t transition_timer = 0;
    bool transition_latched = false;
};

using WorldEffectBoundary = std::function<void()>;
using WorldSoundBoundary = std::function<void(std::uint16_t sound_id)>;

// ROM 0x1E04A: schedule the scene-14 timeout or start the mode-2 world
// effect transition. The sound, pending-flag clear, and effect-list walk are
// explicit host boundaries because their callees are separate source hills.
WorldEffectTransitionAction begin_world_effect_transition(
    WorldEffectTransitionState& state,
    WorldActivityTimeoutState& timeout_state,
    const WorldSoundBoundary& play_sound,
    const WorldEffectBoundary& clear_pending_flags,
    const WorldEffectBoundary& process_effect_list);

struct WorldEffectRegion {
    std::int16_t minimum_x = 0;
    std::int16_t maximum_x = 0;
    std::int16_t minimum_y = 0;
    std::int16_t maximum_y = 0;
    bool sentinel = false;
};

struct WorldEffectTriggerState {
    WorldEffectTransitionState transition{};
    bool trigger_suppressed = false;
    std::uint16_t frame_counter = 0;
    std::uint16_t player_fixed_x = 0;
    std::uint16_t player_fixed_y = 0;
};

struct WorldEffectGuardEntity {
    std::uint8_t behavior_flags = 0;
    std::uint8_t effect_flags = 0;
};

enum class WorldEffectPlayerSpawnGuardAction {
    no_pending_effect,
    existing_effect_found,
    spawn_requested,
};

// ROM 0x286E: if the world-effect marker is set, scan the thirteen entity
// records for the active/effect combination that suppresses another spawn;
// otherwise invoke the player-position spawn boundary.
WorldEffectPlayerSpawnGuardAction guard_world_effect_player_spawn(
    bool world_effect_pending,
    const std::array<WorldEffectGuardEntity, 13>& entities,
    const std::function<void()>& spawn_at_player);

enum class WorldEffectTriggerAction {
    no_action,
    transition_finished,
    trigger_suppressed,
    cadence_wait,
    random_rejected,
    no_matching_region,
    timeout_scheduled,
    transition_already_latched,
    transition_started,
};

using WorldRandomBoundary = std::function<std::uint16_t()>;

// ROM 0x1A342: scan the seven effect-entity slots and return slot 3..9, or
// the source full-table sentinel 10.
std::uint16_t find_first_available_world_effect_entity_slot(
    const std::array<std::uint8_t, 7>& active_flags);

struct WorldEffectSpawnDescriptor {
    std::uint32_t template_key = 0;
    std::uint16_t destination_x = 0;
    std::uint16_t destination_y = 0;
    bool sentinel = false;
    bool reset_vertical_origin = false;
};

struct WorldEffectEntityRecord {
    std::uint16_t entity_slot = 0;
    std::uint16_t effect_slot = 0;
    std::uint16_t vertical_origin = 0;
    std::optional<std::uint16_t> parent_entity_slot;
    std::uint16_t world_x = 0;
    std::uint16_t world_y = 0;
    std::uint16_t transition_code = 0;
    bool overlay_flag = false;
    bool effect_active = false;
    bool visible = false;
};

enum class WorldEffectEntitySpawnAction {
    completed,
    entity_table_full,
    effect_table_full,
};

struct WorldEffectEntitySpawnResult {
    WorldEffectEntitySpawnAction action =
        WorldEffectEntitySpawnAction::completed;
    std::int16_t final_vertical_origin = 0;
    std::vector<WorldEffectEntityRecord> entities;
};

using WorldEffectEntityInitializer = std::function<WorldEffectEntityRecord(
    const WorldEffectSpawnDescriptor& descriptor,
    std::uint16_t entity_slot,
    std::uint16_t effect_slot,
    std::uint16_t vertical_origin,
    const std::optional<std::uint16_t>& parent_entity_slot)>;
using WorldEffectEntityProjection =
    std::function<bool(WorldEffectEntityRecord& entity)>;

using WorldEffectEntitySlotAllocator =
    std::function<std::optional<std::uint16_t>()>;
using WorldEffectRecordBoundary = std::function<void(
    std::uint16_t entity_slot,
    std::uint16_t effect_slot,
    std::uint16_t world_x,
    std::uint16_t world_y,
    bool special_record)>;

enum class WorldEffectSingleSpawnAction {
    spawned,
    entity_table_full,
    effect_table_full,
};

struct WorldEffectSingleSpawnResult {
    WorldEffectSingleSpawnAction action =
        WorldEffectSingleSpawnAction::spawned;
    std::optional<std::uint16_t> entity_slot;
    std::optional<std::uint16_t> effect_slot;
    std::uint16_t world_x = 0;
    std::uint16_t world_y = 0;
    std::uint8_t active_count = 0;
};

struct WorldEffectCoordinateSequenceState {
    std::vector<std::pair<std::uint16_t, std::uint16_t>> coordinates;
    std::pair<std::uint16_t, std::uint16_t> fallback{0, 0};
    std::size_t cursor = 0;
    std::uint8_t active_count = 0;
};

// ROM 0x1E25A: allocate one effect entity from the current coordinate pair,
// using the fallback pair when the first coordinate is zero, then arm the
// world activity timeout.
WorldEffectSingleSpawnResult spawn_world_effect_from_coordinate_sequence(
    WorldEffectCoordinateSequenceState& state,
    const WorldEffectEntitySlotAllocator& allocate_entity_slot,
    const WorldEffectEntitySlotAllocator& allocate_effect_slot,
    const WorldEffectRecordBoundary& initialize_record,
    const WorldEffectBoundary& schedule_timeout);

// ROM 0x1E4AC: allocate one flagged effect entity at the player position
// plus the source horizontal offset.
WorldEffectSingleSpawnResult spawn_world_effect_at_player_position(
    std::uint16_t player_x,
    std::uint16_t player_y,
    const WorldEffectEntitySlotAllocator& allocate_entity_slot,
    const WorldEffectEntitySlotAllocator& allocate_effect_slot,
    const WorldEffectRecordBoundary& initialize_record);

// ROM 0x1E0DC: walk the active world-effect descriptor list, allocate an
// entity/effect slot pair, apply the source vertical-origin grouping, mark
// the initialized entity for effect processing, project it, and hand off the
// descriptor coordinates when it is visible. Entity initialization and
// projection are explicit boundaries for their separate source routines.
WorldEffectEntitySpawnResult spawn_world_effect_entities(
    const std::vector<WorldEffectSpawnDescriptor>& descriptors,
    const std::function<std::optional<std::uint16_t>()>& allocate_entity_slot,
    const std::function<std::optional<std::uint16_t>()>& allocate_effect_slot,
    const WorldEffectEntityInitializer& initialize_entity,
    const WorldEffectEntityProjection& project_entity);

// ROM 0x1DFC4: finish an expired transition or, on the source cadence,
// sample the PRNG and scan the bounded effect-region list before delegating
// to the 0x1E04A transition policy.
WorldEffectTriggerAction advance_world_effect_trigger(
    WorldEffectTriggerState& state,
    std::array<std::uint8_t, kWorldEffectFlagSlotCount>& effect_flags,
    const std::vector<WorldEffectRegion>& regions,
    const WorldRandomBoundary& next_random,
    WorldActivityTimeoutState& timeout_state,
    const WorldSoundBoundary& play_sound,
    const WorldEffectBoundary& finish_transition_sound,
    const WorldEffectBoundary& clear_pending_flags,
    const WorldEffectBoundary& process_effect_list);

} // namespace shadowrun::ghidra_conversion
