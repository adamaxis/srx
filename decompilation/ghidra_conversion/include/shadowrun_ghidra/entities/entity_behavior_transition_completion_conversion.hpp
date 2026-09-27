#pragma once

#include "shadowrun_ghidra/entities/entity_behavior_frame_service_conversion.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct EntityBehaviorLinkedRecord {
    bool active = false; // linked record +0x10 bit 0
    EntityIdentity owner_identity = 0; // linked record +0xA8 back-link
    std::uint8_t behavior_flags = 0; // linked record +0x10 bits
};

struct EntityBehaviorTransitionWorldState {
    // FFE177: the source byte is an unsigned countdown. The decrement wraps
    // at zero exactly as the Genesis byte operation did.
    std::uint8_t scene_spawn_countdown = 0;

    // FFE12E: the source stores a deferred activity handler. The host keeps
    // the pending behavior as a callable instead of preserving a code/data
    // address or reconstructing a function-pointer table.
    std::function<void()> deferred_world_activity;

    // FF0110/FF0210/FF0310: typed field/runner gate flags used by the final
    // activity-countdown branch. Their source offsets are provenance only.
    std::uint8_t field_entity_flags = 0;
    std::uint8_t primary_runner_flags = 0;
    std::uint8_t recruited_runner_flags = 0;

    // FFE166: delay armed when the field and both runner gates are ready.
    std::uint16_t world_activity_countdown = 0;

    // FF0400: the six linked records checked when the completed entity owns
    // a link-propagation transition.
    std::array<EntityBehaviorLinkedRecord, 6> linked_records{};
};

struct EntityBehaviorTransitionCallbacks {
    // ROM 0x1E2EC: begin the activity associated with this entity transition.
    std::function<void()> begin_entity_transition_activity;

    // ROM 0x16B98: populate the next scene entity and update its typed
    // +0x9E completion flags through the returned result.
    std::function<bool(EntityBehaviorFrameRecord&)>
        spawn_scene_entity_from_weighted_table;

    // ROM 0x1DBF0: clear the ten source service records after the spawn
    // countdown reaches zero.
    std::function<void()> reset_entity_service_records;
};

enum class EntityBehaviorTransitionResultKind {
    completed,
    callback_unresolved,
};

struct EntityBehaviorTransitionResult {
    EntityBehaviorTransitionResultKind kind =
        EntityBehaviorTransitionResultKind::completed;
    bool entity_transition_activity_started = false;
    bool scene_entity_spawned = false;
    bool service_records_reset = false;
    bool deferred_activity_started = false;
    std::size_t linked_records_marked = 0;
    bool world_activity_countdown_armed = false;
};

// ROM 0x0AB02: finish one entity behavior transition. This is the concrete
// callee used by 0x0A8EA; it does not emulate the original pointer-backed
// records. The source's flags, counters, six-record link scan, and runner
// readiness gate are represented as typed host state.
EntityBehaviorTransitionResult complete_entity_behavior_transition(
    EntityBehaviorFrameRecord& record,
    EntityBehaviorTransitionWorldState& world,
    const EntityBehaviorTransitionCallbacks& callbacks);

bool entity_behavior_transition_callbacks_complete(
    const EntityBehaviorFrameRecord& record,
    const EntityBehaviorTransitionWorldState& world,
    const EntityBehaviorTransitionCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
