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

    const auto event_code = static_cast<std::uint8_t>(
        state.terrain_collision_code & 0x001FU);
    result.event_code = event_code;

    const auto begin_idle =
        [&](const MovementModeTransitionContinuation continuation,
            const EntityMovementModeTransitionResultKind kind) {
            result.continuation = continuation;
            if (!callbacks.begin_world_activity_if_idle) {
                result.kind =
                    EntityMovementModeTransitionResultKind::callback_unresolved;
                return result;
            }
            callbacks.begin_world_activity_if_idle(continuation);
            return release_record(kind);
        };

    const auto begin_unconditionally =
        [&](const MovementModeTransitionContinuation continuation,
            const EntityMovementModeTransitionResultKind kind) {
            result.continuation = continuation;
            if (!callbacks.begin_world_activity) {
                result.kind =
                    EntityMovementModeTransitionResultKind::callback_unresolved;
                return result;
            }
            callbacks.begin_world_activity(continuation);
            return release_record(kind);
        };

    switch (event_code) {
    case 0x1FU:
        return begin_idle(
            MovementModeTransitionContinuation::event_31_world_activity,
            EntityMovementModeTransitionResultKind::world_activity_started);
    case 0x1EU:
        return begin_idle(
            MovementModeTransitionContinuation::event_30_world_activity,
            EntityMovementModeTransitionResultKind::world_activity_started);
    case 0x1DU:
        if (!state.activity_sequence_active) {
            return begin_idle(
                MovementModeTransitionContinuation::event_29_world_activity,
                EntityMovementModeTransitionResultKind::world_activity_started);
        }
        return release_record(
            EntityMovementModeTransitionResultKind::projection_skipped);
    case 0x1CU:
        if (!state.activity_sequence_active) {
            return begin_idle(
                MovementModeTransitionContinuation::event_28_world_activity,
                EntityMovementModeTransitionResultKind::world_activity_started);
        }
        return release_record(
            EntityMovementModeTransitionResultKind::projection_skipped);
    case 0x1BU:
        return begin_idle(
            MovementModeTransitionContinuation::event_27_world_activity,
            EntityMovementModeTransitionResultKind::world_activity_started);
    case 0x1AU: {
        state.projected_terrain_event_published = false;
        if ((record.projected_field_code & 0x00FFU) != 0x00DAU &&
            !state.world_effect_control_bit) {
            state.projected_terrain_event_published = true;
        }
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
    }
    default:
        break;
    }

    if ((state.scene_id == 0x14U || state.world_mode == 2U) &&
        event_code >= 2U && event_code <= 5U &&
        !state.terrain_event_suppressed) {
        state.activity_source_word = state.world_activity_source_word;
        const auto result = begin_idle(
            MovementModeTransitionContinuation::maglock_door_interaction,
            EntityMovementModeTransitionResultKind::field_dialogue_started);
        // ROM 0xB994 stores the event byte left in D0 by 0x1E2E0. The other
        // begin-activity branches return without touching FFE16E.
        if (result.kind !=
            EntityMovementModeTransitionResultKind::callback_unresolved) {
            state.activity_selection = event_code;
        }
        return result;
    }

    if (event_code == 0x18U) {
        return begin_idle(
            MovementModeTransitionContinuation::building_security_terminal,
            EntityMovementModeTransitionResultKind::building_reaction_started);
    }

    if (event_code == 0x19U) {
        if (state.field_transition_context == 0x13U) {
            return begin_idle(
                MovementModeTransitionContinuation::event_25_world_activity,
                EntityMovementModeTransitionResultKind::world_activity_started);
        }
        state.directional_probe_signs[0] = -0x58;
        state.quantized_field_delta = 0x00A8U;
        state.terrain_collision_code = 0x00A8U;
        record.projected_field_code = state.terrain_collision_code;
        result.event_code = 8U;
    }

    if ((state.field_global_flags & 0x50U) != 0U) {
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
    }

    // ROM 0xBA3A is the only point in this routine that publishes FFFBF9.
    // The early 0x1A/0x18/0x1B..0x1F activity returns intentionally leave the
    // previous global event code intact.
    state.mode_event_code = result.event_code;
    result.mode_event_code_published = true;

    const bool skip_projection =
        state.world_mode == 2U && state.scene_id != 0x14U &&
        result.event_code == 0U &&
        (state.interaction_gate != 0U || state.activity_sequence_active);
    if (skip_projection) {
        return release_record(
            EntityMovementModeTransitionResultKind::projection_skipped);
    }
    if (!callbacks.project_entity_world_interaction) {
        result.kind = EntityMovementModeTransitionResultKind::callback_unresolved;
        return result;
    }
    if (callbacks.project_entity_world_interaction()) {
        state.interaction_transition_pending = true;
        state.interaction_transition_timer = 0x10U;
        state.world_interaction_lock = true;
        result.kind =
            EntityMovementModeTransitionResultKind::projected_interaction_pending;
        return result;
    }
    record.projected_field_code = state.terrain_collision_code;
    result.kind =
        EntityMovementModeTransitionResultKind::projected_field_code_updated;
    return result;
}

} // namespace shadowrun::ghidra_conversion
