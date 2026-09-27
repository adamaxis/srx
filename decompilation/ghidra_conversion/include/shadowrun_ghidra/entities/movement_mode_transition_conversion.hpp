#pragma once

#include "shadowrun_ghidra/entities/entity_behavior_frame_service_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

// The source stores a small direction ordinal in D4 after 0xBC36.  The
// caller-facing record receives the 16-bit rotated heading, so the host
// exposes the ordinal as a value rather than leaking a source register.
struct MovementProjectionPreparation {
    std::uint8_t direction_slot = 0;
};

// These are the continuation families selected by 0xB7B8.  The original
// stores code addresses in FFDD2C; the conversion stores a typed choice and
// lets the field/world layer bind it to a native scene or activity handler.
enum class MovementModeTransitionContinuation {
    none,
    event_31_world_activity, // source continuation 0x0DB26
    event_30_world_activity, // source continuation 0x20990
    event_29_world_activity, // source continuation 0x58D94
    event_28_world_activity, // source continuation 0x58DB2
    event_27_world_activity, // source continuation 0x53D5A
    projected_terrain_event, // source continuation 0x20C90
    field_dialogue_event, // source continuation 0x20E48
    building_reaction_event, // source continuation 0x20EC2
    event_25_world_activity, // source continuation 0x53EFA
    field_mode_exit_finalizer, // native boundary for source 0x0C64C
};

struct EntityMovementModeTransitionState {
    bool world_interaction_lock = false; // FFE8 != 0
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
    std::uint16_t quantized_field_delta = 0; // FFDF64
    bool projected_terrain_event_published = false; // FFE178
    std::uint8_t activity_selection = 0; // FFE16E
    std::uint16_t activity_source_word = 0; // FFE11A
    std::uint16_t world_activity_source_word = 0; // FFDFE4
    std::uint8_t mode_event_code = 0; // FFFBF9
    bool interaction_transition_pending = false; // FFDF3A == -1
    std::uint16_t interaction_transition_timer = 0; // FFDF3C
};

struct EntityMovementModeTransitionCallbacks {
    // ROM 0xBC36: build the three collision-probe points and return D4's
    // selected direction slot. The host implementation owns the probe data.
    std::function<MovementProjectionPreparation(EntityBehaviorFrameRecord&)>
        build_collision_probe_points;

    // ROM 0x423C: quantize the candidate X coordinate against the active
    // field origin. The source also prepares a Y argument, but this helper
    // consumes D0 only; the source-derived X target is passed explicitly.
    std::function<std::int16_t(std::int16_t target,
                               std::uint16_t origin)>
        quantize_field_delta;

    // ROM 0x1E2E0 / 0x1E2EC: bind the typed continuation to an activity
    // scheduler. The idle variant returns the source selection byte.
    std::function<std::uint8_t(MovementModeTransitionContinuation)>
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
    std::uint8_t event_code = 0;
    bool record_released = false;
};

// ROM 0x0B7B8: consume a blocked directional movement probe, derive the
// entity's source heading, quantize its candidate field coordinate, and route
// the resulting event code into a typed world/mode continuation. The host
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
