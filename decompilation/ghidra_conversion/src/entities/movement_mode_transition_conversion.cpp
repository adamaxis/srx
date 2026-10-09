#include "shadowrun_ghidra/entities/movement_mode_transition_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

std::uint16_t rotate_direction_slot_for_entity_heading(
    const std::uint8_t direction_slot) {
    const auto word = static_cast<std::uint16_t>(direction_slot);
    return static_cast<std::uint16_t>((word >> 2U) | (word << 14U));
}

} // namespace

EntityMovementModeTransitionResult
advance_entity_movement_and_mode_transition(
    EntityBehaviorFrameRecord& record,
    const std::uint16_t candidate_x,
    const std::uint16_t candidate_y,
    EntityMovementModeTransitionState& state,
    const EntityMovementModeTransitionCallbacks& callbacks) {
    EntityMovementModeTransitionResult result{};

    // B7B8 is guarded by the transition lock and by its one-shot pending bit.
    if (state.world_interaction_lock) {
        result.kind = EntityMovementModeTransitionResultKind::already_locked;
        return result;
    }
    if ((record.route_flags & 0x20U) != 0U) {
        result.kind = EntityMovementModeTransitionResultKind::already_pending;
        return result;
    }
    record.route_flags = static_cast<std::uint8_t>(record.route_flags | 0x20U);

    const auto release_record =
        [&](const EntityMovementModeTransitionResultKind kind) {
            record.route_flags = static_cast<std::uint8_t>(
                (record.route_flags & static_cast<std::uint8_t>(~0x20U)) |
                0x01U);
            result.kind = kind;
            result.record_released = true;
            return result;
        };

    // The source only attempts the movement-event path for records carrying
    // behavior bit 1 and for mode/scene/interaction combinations that allow
    // the field service to inspect the projected cell.
    if ((record.behavior_flags & 0x02U) == 0U) {
        return release_record(
            EntityMovementModeTransitionResultKind::released_without_behavior);
    }
    if (state.world_mode != 2U && state.scene_id != 0x14U &&
        state.interaction_gate != 0U) {
        return release_record(
            EntityMovementModeTransitionResultKind::released_by_scene_gate);
    }

    std::uint8_t negative_probe_count = 0;
    for (const auto probe : state.directional_probe_signs) {
        if (probe < 0) {
            ++negative_probe_count;
        }
    }
    result.negative_probe_count = negative_probe_count;
    if (negative_probe_count < 2U) {
        return release_record(
            EntityMovementModeTransitionResultKind::released_by_probe_count);
    }

    // 0xBC36 leaves D4 as the selected probe direction. D5/D6 are the
    // candidate coordinates from the preceding directional collision probe;
    // 0x423C quantizes those coordinates, while the event code below is read
    // separately from the raw FFDF64 collision result.
    if (!callbacks.build_collision_probe_points ||
        !callbacks.quantize_field_delta) {
        result.kind = EntityMovementModeTransitionResultKind::callback_unresolved;
        return result;
    }
    const auto projection = callbacks.build_collision_probe_points(record);
    record.action_heading_source =
        rotate_direction_slot_for_entity_heading(projection.direction_slot);
    const auto quantization_target = static_cast<std::uint16_t>(
        (candidate_x & 0xFE00U) + 0x0070U);
    // Keep the source's Y-side preparation explicit in the call boundary even
    // though 0x423C uses D0 only. This prevents the host API from pretending
    // that the second source coordinate was never part of the path.
    const auto quantization_y = static_cast<std::uint16_t>(
        (candidate_y & 0xFE00U) - 0x0190U);
    (void)quantization_y;
    const auto quantized = callbacks.quantize_field_delta(
        static_cast<std::int16_t>(quantization_target), state.field_origin_x);
    state.quantized_field_delta = static_cast<std::uint16_t>(quantized);
    // 0xB858 reloads FFDF64 after the coordinate quantizer returns, then
    // copies that collision result into the entity's +0xC6 field.
    record.projected_field_code = state.terrain_collision_code;

    const auto begin_idle =
        [&](const MovementModeTransitionContinuation continuation,
            const EntityMovementModeTransitionResultKind kind) {
            result.continuation = continuation;
            if (!callbacks.begin_world_activity_if_idle) {
                result.kind =
                    EntityMovementModeTransitionResultKind::callback_unresolved;
                return result;
            }
            if (!callbacks.begin_world_activity_if_idle(continuation)) {
                result.continuation = continuation;
                return release_record(
                    EntityMovementModeTransitionResultKind::
                        world_activity_already_running);
            }
            return release_record(kind);
        };

    if (!callbacks.dispatch_field_map_event) {
        result.kind =
            EntityMovementModeTransitionResultKind::callback_unresolved;
        return result;
    }
    const auto decision_result = callbacks.dispatch_field_map_event(
        FieldMapCollisionEventInput{
            state.scene_id,
            state.world_mode,
            state.field_transition_context,
            state.interaction_gate,
            state.terrain_collision_code,
            candidate_x,
            candidate_y,
            state.field_global_flags,
            state.activity_sequence_active,
            state.terrain_event_suppressed,
            state.world_effect_control_bit});
    if (!decision_result) {
        result.kind = EntityMovementModeTransitionResultKind::
            callback_unresolved;
        return result;
    }
    const auto& decision = *decision_result;
    result.event_code = decision.event_code;
    state.terrain_collision_code = decision.raw_collision_code;
    record.projected_field_code = decision.raw_collision_code;
    if (decision.rewrite_direction_probe) {
        state.directional_probe_signs[0] = decision.direction_probe_value;
    }
    if (decision.rewrite_quantized_delta) {
        state.quantized_field_delta = decision.quantized_delta;
    }
    if (decision.action == FieldMapEventAction::mark_projected_terrain) {
        state.projected_terrain_event_published =
            decision.projected_terrain_event_published;
    }
    if (decision.mode_event_code_published) {
        // This is the only policy action that publishes FFFBF9. Early source
        // continuations intentionally leave its previous value untouched.
        state.mode_event_code = field_collision_event_value(decision.event_code);
        result.mode_event_code_published = true;
    }

    switch (decision.action) {
    case FieldMapEventAction::begin_event_31_activity:
        return begin_idle(
            MovementModeTransitionContinuation::event_31_world_activity,
            EntityMovementModeTransitionResultKind::world_activity_started);
    case FieldMapEventAction::begin_event_30_activity:
        return begin_idle(
            MovementModeTransitionContinuation::event_30_world_activity,
            EntityMovementModeTransitionResultKind::world_activity_started);
    case FieldMapEventAction::begin_event_29_activity:
        return begin_idle(
            MovementModeTransitionContinuation::event_29_world_activity,
            EntityMovementModeTransitionResultKind::world_activity_started);
    case FieldMapEventAction::begin_event_28_activity:
        return begin_idle(
            MovementModeTransitionContinuation::event_28_world_activity,
            EntityMovementModeTransitionResultKind::world_activity_started);
    case FieldMapEventAction::begin_event_27_activity:
        return begin_idle(
            MovementModeTransitionContinuation::event_27_world_activity,
            EntityMovementModeTransitionResultKind::world_activity_started);
    case FieldMapEventAction::mark_projected_terrain:
        if (!callbacks.mark_projected_terrain_cell) {
            result.kind =
                EntityMovementModeTransitionResultKind::callback_unresolved;
            return result;
        }
        if (!callbacks.mark_projected_terrain_cell()) {
            return release_record(
                EntityMovementModeTransitionResultKind::terrain_cell_not_marked);
        }
        result.continuation =
            MovementModeTransitionContinuation::projected_terrain_event;
        if (!callbacks.begin_world_activity) {
            result.kind =
                EntityMovementModeTransitionResultKind::callback_unresolved;
            return result;
        }
        callbacks.begin_world_activity(result.continuation);
        return release_record(
            EntityMovementModeTransitionResultKind::projected_terrain_marked);
    case FieldMapEventAction::begin_maglock_interaction: {
        state.activity_source_word = state.world_activity_source_word;
        const auto started = begin_idle(
            MovementModeTransitionContinuation::maglock_door_interaction,
            EntityMovementModeTransitionResultKind::field_dialogue_started);
        // ROM 0xB994 stores the event byte left in D0 by 0x1E2E0.
        if (started.kind !=
            EntityMovementModeTransitionResultKind::callback_unresolved) {
            state.activity_selection =
                field_collision_event_value(decision.event_code);
        }
        return started;
    }
    case FieldMapEventAction::begin_security_terminal:
        return begin_idle(
            MovementModeTransitionContinuation::building_security_terminal,
            EntityMovementModeTransitionResultKind::building_reaction_started);
    case FieldMapEventAction::begin_event_25_activity:
        return begin_idle(
            MovementModeTransitionContinuation::event_25_world_activity,
            EntityMovementModeTransitionResultKind::world_activity_started);
    case FieldMapEventAction::begin_field_mode_exit:
        result.continuation =
            MovementModeTransitionContinuation::field_mode_exit_finalizer;
        if (!callbacks.prepare_mode_entry_service ||
            !callbacks.begin_world_activity) {
            result.kind =
                EntityMovementModeTransitionResultKind::callback_unresolved;
            return result;
        }
        callbacks.prepare_mode_entry_service();
        callbacks.begin_world_activity(result.continuation);
        return release_record(
            EntityMovementModeTransitionResultKind::mode_exit_started);
    case FieldMapEventAction::skip_world_interaction:
        return release_record(
            EntityMovementModeTransitionResultKind::projection_skipped);
    case FieldMapEventAction::project_world_interaction:
        if (!callbacks.project_entity_world_interaction) {
            result.kind =
                EntityMovementModeTransitionResultKind::callback_unresolved;
            return result;
        }
        if (callbacks.project_entity_world_interaction()) {
            state.interaction_transition_pending = true;
            state.interaction_transition_timer = 0x10U;
            state.world_interaction_lock = true;
            result.kind = EntityMovementModeTransitionResultKind::
                projected_interaction_pending;
            return result;
        }
        record.projected_field_code = state.terrain_collision_code;
        result.kind = EntityMovementModeTransitionResultKind::
            projected_field_code_updated;
        return result;
    }

    result.kind = EntityMovementModeTransitionResultKind::callback_unresolved;
    return result;
}

} // namespace shadowrun::ghidra_conversion
