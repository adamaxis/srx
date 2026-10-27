#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct EntityProximityRecord {
    std::uint16_t x = 0;                    // source record +0x00
    std::uint16_t y = 0;                    // source record +0x04
    std::uint8_t entity_flags = 0;          // source record +0x10
    std::uint8_t collision_flags = 0;       // source record +0x11
    std::uint8_t special_flags = 0;          // source record +0x9E
    std::uint8_t activity_flags = 0;         // source record +0xB0
    std::uint16_t behavior_value = 0;       // source record +0x38
    std::optional<std::size_t> primary_link;   // source record +0xA2
    std::optional<std::size_t> secondary_link; // source record +0xAC
    std::uint16_t interaction_heading = 0;  // source record +0x92
    std::uint16_t interaction_value = 0;    // source record +0x94
    bool player_record = false;
};

struct EntityProximityInteractionState {
    std::size_t pairs_examined = 0;
    std::size_t pairs_resolved = 0;
    bool skipped_current_record = false;
};

struct EntityProximityInteractionCallbacks {
    std::function<void(EntityProximityRecord& current,
                       EntityProximityRecord& candidate)>
        resolve_pair_collision;
    std::function<std::uint16_t()> advance_random;
};

// ROM 0x0EB92: scan later active entity records through FF0B00, use the
// linked-record radius when either source relationship points at the other
// record, set collision flags for an in-range pair, publish opposing
// interaction headings/values, then route the pair through 0xECCC. A zero
// 0xD44 & 0x7000 result ADDI.W into +0x92 only; +0x94 ADD.W is D0 scratch.
EntityProximityInteractionState resolve_entity_proximity_interactions(
    std::vector<EntityProximityRecord>& records,
    std::size_t current_index,
    const EntityProximityInteractionCallbacks& callbacks = {});

enum class EntityPairActor {
    current,
    candidate,
};

enum class EntityPairActivityHandler {
    none,
    special_collision,
    special_mode_six_collision,
    special_scene_table_collision,
    normal_collision,
    world_effect_collision,
    pending_world_effect_collision,
};

struct EntityPairCollisionEntity {
    std::uint8_t special_flags = 0; // source record +0x9E
    std::uint8_t entity_flags = 0;   // source record +0x10
    std::uint8_t activity_flags = 0; // source record +0xB0
};

// ROM 0xEEF6: signed (scene-10) bytes before the 0xEEFE table. Scene 2
// indexes -8 and reads 0x4E from the previous function's RTS.
constexpr std::array<std::uint8_t, 8> kEntityPairSceneActivityPreamble{
    0x4E, 0x75, 0x51, 0xCF, 0xFF, 0xE4, 0x60, 0xD6};

// ROM 0xEEFE: MOVE.B (A2,D0) after SUBI.W #0xA of FFDDC4. Scene 10 is the
// Z path and does not index this table.
constexpr std::array<std::uint8_t, 10> kEntityPairSceneActivityTable{
    0x04, 0x01, 0x00, 0x06, 0x03, 0x09, 0x05, 0x07, 0x08, 0x00};

struct EntityPairCollisionWorldState {
    bool activity_in_progress = false;       // FFE176
    bool world_effect_active = false;         // FFE180
    bool pending_world_effect = false;        // FFE182
    bool transfer_actor_template_available = false; // Typed FFE110 value
    std::uint16_t scene_activity_id = 0;     // FFDDC4
    std::uint8_t field_mode = 0;              // FFFBF9
    std::vector<std::uint8_t> scene_activity_table;

    std::optional<EntityPairActor> selected_activity_actor;
    EntityPairActivityHandler handler = EntityPairActivityHandler::none;
    std::uint8_t activity_variant = 0;
    bool published_transfer_actor_template_available = false;
    bool world_activity_started = false;
};

struct EntityPairCollisionCallbacks {
    std::function<void(EntityPairActor)> begin_world_activity_if_idle;
    std::function<void()> begin_world_activity;
};

struct EntityPairCollisionResult {
    bool special_activity_requested = false;
    bool normal_activity_requested = false;
    std::optional<EntityPairActor> flagged_entity;
    EntityPairActivityHandler handler = EntityPairActivityHandler::none;
    std::uint8_t activity_variant = 0;
};

// ROM 0x0ECCC: route the pair's special flags through the idle activity gate,
// set the eligible entity's one-shot activity bit for the ordinary path, and
// select the world-effect/pending-effect handler before beginning activity.
// Special-path FFF0AE is scene 10's 4/2 split or signed (scene-10) into
// 0xEEFE; the handler enum is the host equivalent of the source tokens.
EntityPairCollisionResult resolve_entity_pair_collision_event(
    EntityPairCollisionEntity& current,
    EntityPairCollisionEntity& candidate,
    EntityPairCollisionWorldState& world,
    const EntityPairCollisionCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
