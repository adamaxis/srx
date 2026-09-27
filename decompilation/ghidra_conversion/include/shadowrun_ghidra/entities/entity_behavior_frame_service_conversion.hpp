#pragma once

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

// A stable host identity replaces the source's A0 pointer comparison. It is
// an entity key owned by the conversion/runtime boundary, never a ROM or RAM
// address.
using EntityIdentity = std::uint32_t;

enum class EntityBehaviorContinuationKind {
    ordinary,
    transition_b0d4,
    transition_b10c,
    fallback_movement,
    collision_recheck,
    movement_route,
};

// The fields here are the confirmed consumers of one 0x100-byte entity slot
// in ROM 0x0A8EA. They are typed gameplay state, not a byte-addressed record;
// the source offsets remain only as provenance comments.
struct EntityBehaviorFrameRecord {
    bool active = false; // +0x10 bit 0
    std::uint8_t behavior_flags = 0; // +0x10 bits 1..6
    std::uint8_t record_flags = 0; // +0x11 bits 0..2
    std::int16_t animation_elapsed = 0; // +0x16
    std::uint16_t entity_type = 0; // +0x0E
    std::uint16_t current_heading = 0; // +0x2E
    std::uint16_t target_heading = 0; // +0x34
    std::uint16_t action_heading_source = 0; // +0xCC
    std::uint16_t projected_field_code = 0; // +0xC6
    std::int16_t transition_gate = 0; // +0xDC
    std::int8_t target_marker = 0; // +0x56
    std::uint8_t source_type = 0; // +0x57
    std::uint8_t auxiliary_marker = 0; // +0x43
    std::uint8_t transition_rate_threshold = 0; // +0x9C
    std::uint8_t transition_gate_marker = 0; // +0x4C
    std::uint8_t route_flags = 0; // +0xB0
    std::uint8_t spawn_state_flags = 0; // +0x9E, consumed by ROM 0xAB02
    EntityIdentity identity = 0; // A0 identity, used by typed link matching
    std::uint8_t action_result = 0; // +0xB3
    EntityBehaviorContinuationKind continuation_kind =
        EntityBehaviorContinuationKind::ordinary;
    EntityBehaviorContinuationKind saved_continuation_kind =
        EntityBehaviorContinuationKind::ordinary;
};

using EntityBehaviorFrameBoundary =
    std::function<void(EntityBehaviorFrameRecord&)>;

struct EntityBehaviorFrameCallbacks {
    // ROM 0x13874 runs once before the sixteen-slot scan.
    std::function<void()> dispatch_entity_action_candidates;

    // ROM 0x15BB4 returns the action-result byte stored at +0xB3.
    std::function<std::uint8_t(EntityBehaviorFrameRecord&)>
        noop_entity_action_hook;

    EntityBehaviorFrameBoundary complete_entity_behavior_transition;
    EntityBehaviorFrameBoundary clear_effect_slot_state;
    EntityBehaviorFrameBoundary handle_mode_action_event;
    EntityBehaviorFrameBoundary advance_entity_behavior_action;
    EntityBehaviorFrameBoundary enter_entity_behavior_transition;
    EntityBehaviorFrameBoundary dispatch_entity_collision_behavior;
    EntityBehaviorFrameBoundary apply_mode_five_boundary_motion;
    EntityBehaviorFrameBoundary resolve_entity_proximity_interactions;
    EntityBehaviorFrameBoundary clear_entity_effect_if_forced;
    EntityBehaviorFrameBoundary resolve_entity_fallback_movement;

    // The source calls the record's dynamic +0x3A continuation after clearing
    // its pending bit. The callback is the host representation of that call.
    EntityBehaviorFrameBoundary run_entity_continuation;
};

enum class EntityBehaviorFrameServiceResult {
    completed,
    callback_unresolved,
};

// ROM 0x0A8EA: dispatch action candidates, then service active entity slots
// in source order. DBF D7 executes exactly sixteen records. The transition,
// movement, proximity, animation, and action callees remain explicit typed
// boundaries; no function-pointer or Genesis-RAM address is exposed.
EntityBehaviorFrameServiceResult service_entity_behavior_frames(
    std::array<EntityBehaviorFrameRecord, 16>& records,
    const EntityBehaviorFrameCallbacks& callbacks);

bool entity_behavior_frame_callbacks_complete(
    const EntityBehaviorFrameCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
