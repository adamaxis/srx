#pragma once

#include "shadowrun_ghidra/entities/entity_behavior_frame_service_conversion.hpp"
#include "shadowrun_ghidra/field/field_map_event_system_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

// The source stores a small direction ordinal in D4 after 0xBC36.  The
// caller-facing record receives the 16-bit rotated heading, so the host
// exposes the ordinal as a value rather than leaking a source register.
struct MovementProjectionPreparation {
    std::uint8_t direction_slot = 0;
};

// These are typed field/activity continuations. Most are selected by
// 0xB7B8; Scene 55 is queued by the outer scene selector at 0x0A44. The source
// stores code addresses in FFDD2C; the host stores a typed choice instead.
enum class MovementModeTransitionContinuation {
    none,
    event_31_world_activity, // source continuation 0x0DB26
    event_30_world_activity, // source continuation 0x20990
    event_29_world_activity, // source continuation 0x58D94
    event_28_world_activity, // source continuation 0x58DB2
    event_27_world_activity, // source continuation 0x53D5A
    projected_terrain_event, // source continuation 0x20C90
    world_activity_extraction_result, // source callback 0x1F16A
    maglock_door_interaction, // source continuation 0x20E48
    building_security_terminal, // source continuation 0x20EC2
    event_25_world_activity, // source continuation 0x53EFA
    scene_37_antechamber_entry, // Scene-55 entry callback
    scene_55_tomb_credits_callback, // source callback 0x5BAC4
    mode_table_world_activity, // source-selected field activity during 0x2CE0
    field_mode_exit_finalizer, // native boundary for source 0x0C64C
    guard_spotted_field_event, // source event 0x20FEC queued by 0x1E2E0
};

struct EntityMovementModeTransitionState {
    bool world_interaction_lock = false; // FFDFE8 != 0
    std::uint16_t world_mode = 0; // FFDF40
    std::uint16_t scene_id = 0; // FFDDC4
    std::uint16_t field_transition_context = 0; // FFFC10
    std::uint16_t interaction_gate = 0; // FFDF84
    std::uint8_t field_global_flags = 0; // FF0110
    bool activity_sequence_active = false; // FFE16F
    bool terrain_event_suppressed = false; // FFE17F
    bool world_effect_control_bit = false; // FF FC1C bit 7
    std::array<std::int8_t, 4> directional_probe_signs{}; // FFEF10..13
    std::uint16_t field_origin_x = 0; // FFDDC6
    std::uint16_t quantized_field_delta = 0; // return value from 0x423C
    std::uint16_t terrain_collision_code = 0; // FFDF64, consumed by 0xB7B8
    bool projected_terrain_event_published = false; // FFE178
    std::uint8_t activity_selection = 0; // FFE16E
    std::uint16_t activity_source_word = 0; // FFE11A
    std::uint16_t world_activity_source_word = 0; // FFDFE4
    std::uint8_t mode_event_code = 0; // FFFBF9
    bool interaction_transition_pending = false; // FFDF3A == -1
    std::int16_t interaction_transition_timer = 0; // signed FFDF3C
};

struct EntityMovementModeTransitionCallbacks {
    // ROM 0xBC36: build the three collision-probe points and return D4's
    // selected direction slot. The host implementation owns the probe data.
    std::function<MovementProjectionPreparation(EntityBehaviorFrameRecord&)>
        build_collision_probe_points;

    // ROM 0x423C: quantize the candidate X coordinate against the active
    // field origin. Its return is separate from FFDF64, which B7B8 reads for
    // the entity collision code and low-five-bit event selection.
    std::function<std::int16_t(std::int16_t target,
                               std::uint16_t origin)>
        quantize_field_delta;

    // The Lua field-event policy receives every accepted collision event.
    // Native movement executes its typed result through engine services.
    std::function<std::optional<FieldMapEventPolicyDecision>(
        const FieldMapCollisionEventInput&)>
        dispatch_field_map_event;

    // ROM 0x1E2E0 / 0x1E2EC: bind the typed continuation to an activity
    // scheduler. The source routine does not return a selection value; the
    // bool only reports whether the host timer was started for this call.
    std::function<bool(MovementModeTransitionContinuation)>
        begin_world_activity_if_idle;
    std::function<void(MovementModeTransitionContinuation)>
        begin_world_activity;

    // ROM 0x16760 / 0x1651E: these callbacks return the source carry result.
    std::function<bool()> mark_projected_terrain_cell;
    std::function<bool()> project_entity_world_interaction;

    // ROM 0x53126: prepare the mode-entry service before the field exit
    // continuation is scheduled.
    std::function<void()> prepare_mode_entry_service;
};

enum class EntityMovementModeTransitionResultKind {
    callback_unresolved,
    already_locked,
    already_pending,
    released_without_behavior,
    released_by_scene_gate,
    released_by_probe_count,
    world_activity_already_running,
    world_activity_started,
    projected_terrain_marked,
    terrain_cell_not_marked,
    field_dialogue_started,
    building_reaction_started,
    mode_exit_started,
    projected_interaction_pending,
    projected_field_code_updated,
    projection_skipped,
};

struct EntityMovementModeTransitionResult {
    EntityMovementModeTransitionResultKind kind =
        EntityMovementModeTransitionResultKind::callback_unresolved;
    MovementModeTransitionContinuation continuation =
        MovementModeTransitionContinuation::none;
    std::uint8_t negative_probe_count = 0;
    FieldCollisionEventCode event_code = FieldCollisionEventCode::event_0;
    bool mode_event_code_published = false;
    bool record_released = false;
};

// ROM 0x0B7B8: consume a blocked directional movement probe, derive the
// entity's source heading, quantize its candidate field coordinate, and route
// FFDF64's collision code into a typed world/mode continuation. The host
// record and callbacks replace A0, FF globals, and the source code pointer in
// FFDD2C; no ROM address is read or executed by this function.
EntityMovementModeTransitionResult
advance_entity_movement_and_mode_transition(
    EntityBehaviorFrameRecord& record,
    std::uint16_t candidate_x,
    std::uint16_t candidate_y,
    EntityMovementModeTransitionState& state,
    const EntityMovementModeTransitionCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
