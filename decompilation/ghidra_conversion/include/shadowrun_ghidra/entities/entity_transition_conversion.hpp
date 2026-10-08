#pragma once

#include "shadowrun_ghidra/entities/behavior_animation_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_combat_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_state_primitives_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct EntityBehaviorSignature {
    std::uint8_t primary = 0;
    std::uint8_t secondary = 0;
};

struct SpawnedBehaviorEntity {
    std::uint16_t resource_id = 0;
    std::uint8_t render_flags = 0;
    std::uint8_t lifecycle_flags = 0;
    std::uint16_t animation = 0;
    std::uint8_t behavior_type = 0;
    std::uint16_t position_x = 0;
    std::uint16_t position_y = 0;
    std::uint16_t vertical_offset = 0;
    EntityBehaviorContinuation continuation =
        EntityBehaviorContinuation::ImmediateCollision;
};

struct ChildAllocation {
    std::uint16_t resource_id = 0;
    SpawnedBehaviorEntity child{};
};

using SignatureRefresh = std::function<void(EntityBehaviorSignature&)>;
using ChildAllocator = std::function<std::optional<ChildAllocation>()>;

struct SpawnEntityParentState {
    std::uint16_t position_x = 0;
    std::uint16_t position_y = 0;
    std::uint16_t heading = 0;
    std::uint8_t flags = 0;
    std::uint8_t field_0x56 = 0;
    std::uint8_t field_0x57 = 0;
    std::uint8_t field_0x9d = 0;
};

struct SpawnEntityLinkedState {
    std::uint16_t position_x = 0;
    std::uint16_t position_y = 0;
    std::uint8_t flags = 0;
    bool parent_backlink_set = false;
};

struct SpawnEntityRecordState {
    EntityBehaviorRecordState behavior_defaults{};
    std::uint16_t position_x = 0;
    std::uint16_t position_y = 0;
    std::uint16_t heading = 0;
    std::uint16_t movement_scale = 0;
    std::uint16_t source_identifier = 0;
    std::uint16_t behavior_code = 0;
    std::uint8_t flags = 0;
    std::uint8_t field_0x56 = 0;
    std::uint8_t field_0x57 = 0;
    std::uint8_t field_0x9d = 0;
    std::int16_t heading_offset_x = 0;
    std::int16_t heading_offset_y = 0;
    std::uint16_t directional_probe_x = 0;
    std::uint16_t directional_probe_y = 0;
    std::uint16_t slot_identifier = 0;
    std::uint8_t lifecycle_state = 0;
    std::uint16_t saved_position_x = 0;
    std::uint16_t saved_position_y = 0;
    std::uint8_t field_0x89 = 0;
    std::uint8_t field_0x8b = 0;
    std::uint32_t continuation_address = 0;
    std::uint8_t source_marker = 0;
    std::uint16_t animation_resource = 0;
    std::uint8_t record_flags = 0;
    bool has_parent_link = false;
};

using SpawnPreparationBoundary = std::function<void()>;

// ROM 0x14FF8: initialize a newly allocated child record from its parent,
// optionally resolve a live linked target, apply the source placement tables,
// and derive its signed heading offsets. Resource allocation and the source
// global preparation call remain explicit host boundaries.
void initialize_entity_spawn_record(
    SpawnEntityRecordState& child,
    const SpawnEntityParentState& parent,
    std::uint16_t source_identifier,
    bool collision_gate_active,
    std::optional<SpawnEntityLinkedState>& linked_target,
    const SpawnPreparationBoundary& prepare_spawn);

// ROM 0x152FA: put the first spawned record into the special follow-up state,
// preserving its current position for the later transition.
void prepare_special_spawn_record(SpawnEntityRecordState& record);

enum class EntitySpawnCycleAction {
    global_mode_blocked,
    animation_window_expired,
    behavior_gate_blocked,
    ammo_service_unresolved,
    ammo_service_inactive,
    no_primary_slot,
    no_secondary_slot,
    invalid_source_type,
    spawned,
};

struct EntitySpawnCycleState {
    SpawnEntityParentState parent{};
    EntityEquipmentState equipment{};
    std::optional<SpawnEntityLinkedState> linked_target;
    std::uint16_t animation_counter = 0;
    std::uint8_t global_mode = 0;
    bool collision_gate_active = false;
    // The byte at A0 + 0x10. Bit 1 controls the late-window overflow latch.
    std::uint8_t entity_flags = 0;
    // Source status byte used by ROM 0x1514E. Bits 1/2 select the alternate
    // audio-event table and drive the world-effect marker at FFDD6D.
    std::uint8_t source_status_flags = 0;
};

using EntitySpawnCycleGate = std::function<bool()>;
using EntitySpawnCycleSlotAllocator =
    std::function<std::optional<std::uint16_t>()>;
using EntitySpawnCycleEffect = std::function<void()>;
using EntitySpawnCycleAudioEffect = std::function<void(std::uint16_t)>;
using EntitySpawnCycleModeEffect = std::function<void(std::uint16_t)>;
using EntitySpawnCycleAmmoService =
    std::function<EntityAmmoServiceAction(EntityEquipmentState&, std::uint8_t)>;

// ROM 0x15318/0x15336: select the sound request for an equipped action code
// and its source inventory phase flags. This is also used by the live field
// input route before the spawned-record presentation lifecycle is complete.
std::optional<std::uint16_t> select_entity_spawn_audio_event(
    std::uint8_t source_action_code,
    std::uint8_t source_status_flags);

struct EntitySpawnCycleResult {
    EntitySpawnCycleAction action =
        EntitySpawnCycleAction::global_mode_blocked;
    std::optional<SpawnEntityRecordState> primary;
    std::optional<SpawnEntityRecordState> secondary;
    std::optional<std::uint16_t> primary_slot;
    std::optional<std::uint16_t> secondary_slot;
    std::uint16_t selected_audio_event = 0;
    EntityAmmoServiceAction ammo_service_action =
        EntityAmmoServiceAction::no_action;
    bool overflow_latch_set = false;
    bool special_follow_up = false;
};

// ROM 0x1514E: run the source entity's spawn window. The source services ammo
// after its gates and before allocating either record. Ammo service, allocator,
// world refresh, audio dispatch, and mode-maintenance calls are explicit host
// boundaries; record initialization, clone, placement, and direction
// selection are represented as typed native state.
EntitySpawnCycleResult process_entity_spawn_cycle(
    EntitySpawnCycleState& state,
    const EntitySpawnCycleGate& behavior_gate,
    const EntitySpawnCycleSlotAllocator& allocate_slot,
    const SpawnPreparationBoundary& prepare_spawn,
    const EntitySpawnCycleEffect& refresh_world,
    const EntitySpawnCycleAudioEffect& play_audio_event,
    const EntitySpawnCycleModeEffect& apply_mode_maintenance,
    const EntitySpawnCycleAmmoService& service_ammo = {});

void initialize_spawned_behavior_entity(
    SpawnedBehaviorEntity& child,
    std::uint16_t resource_id,
    std::uint16_t parent_x,
    std::uint16_t parent_y);

enum class EntityTransitionContinuation {
    SpawnCollisionCheck,
    TargetCollisionCheck,
};

struct EntityTransitionState {
    std::uint8_t type = 0;
    std::uint8_t subtype = 0;
    std::uint16_t position_x = 0;
    std::uint16_t position_y = 0;
    std::uint16_t heading = 0;
    std::uint16_t desired_heading = 0;
    std::uint16_t direction_animation = 0;
    EntityTransitionContinuation continuation =
        EntityTransitionContinuation::TargetCollisionCheck;
    EntityDelayState delay{};
    EntityAnimationResetState animation{};
};

// ROM 0xB11C: refresh a related entity signature, optionally prepare its
// behavior and allocate a child, then move the current entity into the C042
// spawn-collision transition.
std::optional<SpawnedBehaviorEntity> synchronize_entity_behavior_spawn(
    EntityTransitionState& entity,
    EntityBehaviorSignature& related_signature,
    const SignatureRefresh& refresh_signature,
    const EntityBehaviorEffect& prepare_related_behavior,
    const ChildAllocator& allocate_child,
    const EntityDelayAttributes& delay_attributes,
    const std::array<std::uint16_t, 8>& direction_animations,
    const EntityBehaviorEffect& post_spawn_effect);

// ROM 0xB1DC: apply the initial behavior action, resolve a linked heading when
// present, enter the C0A2 target-collision transition, then optionally perform
// the follow-up action and related-entity spawn path.
std::optional<SpawnedBehaviorEntity> configure_entity_target_transition(
    EntityTransitionState& entity,
    const EntityDelayAttributes& delay_attributes,
    bool has_linked_entity,
    const std::function<std::uint16_t()>& resolve_linked_heading,
    const EntityBehaviorAction& behavior_action,
    const std::array<std::uint16_t, 8>& direction_animations,
    const std::function<bool()>& transition_check,
    bool alternate_follow_up_action,
    EntityBehaviorSignature& related_signature,
    const SignatureRefresh& refresh_signature,
    const EntityBehaviorEffect& prepare_related_behavior,
    const ChildAllocator& allocate_child,
    const EntityBehaviorEffect& post_transition_effect);

} // namespace shadowrun::ghidra_conversion
