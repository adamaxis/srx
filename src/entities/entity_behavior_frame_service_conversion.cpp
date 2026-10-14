#include "shadowrun_ghidra/entities/entity_behavior_frame_service_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

bool has_required_callbacks(const EntityBehaviorFrameCallbacks& callbacks) {
    return static_cast<bool>(callbacks.dispatch_entity_action_candidates) &&
           static_cast<bool>(callbacks.noop_entity_action_hook) &&
           static_cast<bool>(callbacks.complete_entity_behavior_transition) &&
           static_cast<bool>(callbacks.clear_effect_slot_state) &&
           static_cast<bool>(callbacks.handle_mode_action_event) &&
           static_cast<bool>(callbacks.advance_entity_behavior_action) &&
           static_cast<bool>(callbacks.enter_entity_behavior_transition) &&
           static_cast<bool>(callbacks.dispatch_entity_collision_behavior) &&
           static_cast<bool>(callbacks.apply_mode_five_boundary_motion) &&
           static_cast<bool>(callbacks.resolve_entity_proximity_interactions) &&
           static_cast<bool>(callbacks.clear_entity_effect_if_forced) &&
           static_cast<bool>(callbacks.resolve_entity_fallback_movement) &&
           static_cast<bool>(callbacks.run_entity_continuation);
}

void complete_transition_and_maybe_handle_action(
    EntityBehaviorFrameRecord& record,
    const EntityBehaviorFrameCallbacks& callbacks) {
    callbacks.complete_entity_behavior_transition(record);
    if ((record.behavior_flags & 0x02U) != 0U) {
        callbacks.handle_mode_action_event(record);
    }
}

void dispatch_action_or_collision(
    EntityBehaviorFrameRecord& record,
    const EntityBehaviorFrameCallbacks& callbacks) {
    if ((record.behavior_flags & 0x02U) != 0U) {
        callbacks.advance_entity_behavior_action(record);
    } else if ((record.behavior_flags & 0x04U) != 0U) {
        callbacks.dispatch_entity_collision_behavior(record);
    }
}

void route_transition_for_rate_gate(
    EntityBehaviorFrameRecord& record,
    const EntityBehaviorFrameCallbacks& callbacks,
    const bool hold_type_c_eighty_transition) {
    // The source reaches AA54 when the secondary +0x4C marker is nonzero.
    // That path dispatches action/collision behavior without setting bit 4.
    if (record.transition_rate_threshold != 0U &&
        record.transition_gate_marker != 0U) {
        dispatch_action_or_collision(record, callbacks);
        return;
    }

    const bool was_transition_started =
        (record.behavior_flags & 0x10U) != 0U;
    record.behavior_flags = static_cast<std::uint8_t>(
        record.behavior_flags | 0x10U);
    if (!was_transition_started) {
        callbacks.enter_entity_behavior_transition(record);
        return;
    }
    if (record.entity_type == 0x000EU) {
        callbacks.complete_entity_behavior_transition(record);
        callbacks.clear_effect_slot_state(record);
        return;
    }
    if (hold_type_c_eighty_transition &&
        record.transition_rate_threshold != 0U &&
        record.entity_type == 0x000CU &&
        record.animation_elapsed == static_cast<std::int16_t>(0x0080)) {
        return;
    }
    complete_transition_and_maybe_handle_action(record, callbacks);
}

} // namespace

EntityBehaviorFrameServiceResult service_entity_behavior_frames(
    std::array<EntityBehaviorFrameRecord, 16>& records,
    const EntityBehaviorFrameCallbacks& callbacks) {
    if (!has_required_callbacks(callbacks)) {
        return EntityBehaviorFrameServiceResult::callback_unresolved;
    }

    callbacks.dispatch_entity_action_candidates();
    for (auto& record : records) {
        if (!record.active) {
            continue;
        }

        if (record.transition_gate >= 0) {
            --record.transition_gate;
        }

        if ((record.behavior_flags & 0x04U) == 0U) {
            if ((record.behavior_flags & 0x02U) != 0U) {
                callbacks.advance_entity_behavior_action(record);
            }
        } else if ((record.behavior_flags & 0x40U) == 0U) {
            if ((record.record_flags & 0x10U) != 0U) {
                record.action_result = callbacks.noop_entity_action_hook(record);
            }

            if (record.animation_elapsed >= 0) {
                if (record.animation_elapsed <= 0x003CU) {
                    route_transition_for_rate_gate(record, callbacks, true);
                }
            } else if (record.continuation_kind ==
                           EntityBehaviorContinuationKind::transition_b0d4 ||
                       record.continuation_kind ==
                           EntityBehaviorContinuationKind::transition_b10c) {
                callbacks.complete_entity_behavior_transition(record);
                callbacks.clear_effect_slot_state(record);
            } else {
                route_transition_for_rate_gate(record, callbacks, true);
            }
        }

        if ((record.behavior_flags & 0x04U) != 0U) {
            callbacks.apply_mode_five_boundary_motion(record);
            callbacks.resolve_entity_proximity_interactions(record);

            const bool fallback_requested =
                (record.record_flags & 0x02U) != 0U;
            record.record_flags = static_cast<std::uint8_t>(
                record.record_flags & ~0x02U);
            if (fallback_requested) {
                record.saved_continuation_kind = record.continuation_kind;
                record.continuation_kind =
                    EntityBehaviorContinuationKind::fallback_movement;
                callbacks.resolve_entity_fallback_movement(record);
                record.continuation_kind = record.saved_continuation_kind;
            }
        }

        record.record_flags = static_cast<std::uint8_t>(
            record.record_flags & ~0x04U);
        const auto previous_animation_elapsed = record.animation_elapsed;
        callbacks.run_entity_continuation(record);
        callbacks.clear_entity_effect_if_forced(record);
        if (record.animation_elapsed != previous_animation_elapsed) {
            record.record_flags = static_cast<std::uint8_t>(
                record.record_flags | 0x01U);
        }
    }

    return EntityBehaviorFrameServiceResult::completed;
}

bool entity_behavior_frame_callbacks_complete(
    const EntityBehaviorFrameCallbacks& callbacks) {
    return has_required_callbacks(callbacks);
}

} // namespace shadowrun::ghidra_conversion
