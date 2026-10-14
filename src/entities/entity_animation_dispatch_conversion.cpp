#include "shadowrun_ghidra/entities/entity_animation_dispatch_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

void mark_missing(EntityAnimationDispatchResult& result) {
    result.boundary_unresolved = true;
}

bool run_resource_gate(const EntityAnimationDispatchCallbacks& callbacks,
                       EntityAnimationDispatchResult& result) {
    if (!callbacks.resource_gate) {
        mark_missing(result);
        return false;
    }
    return callbacks.resource_gate();
}

EntityAnimationDispatchResult source_failure(
    EntityAnimationDispatchResult result,
    const EntityAnimationDispatchAction action) {
    result.action = action;
    result.source_failure_boundary = true;
    return result;
}

void apply_heading(const EntityAnimationHeadingBoundary& boundary,
                   const std::uint16_t heading,
                   EntityAnimationDispatchResult& result) {
    if (boundary) {
        boundary(heading);
        result.applied_heading = heading;
    } else {
        mark_missing(result);
    }
}

EntityAnimationDispatchResult enter_target_motion(
    EntityAnimationDispatchState& state,
    const EntityAnimationDispatchCallbacks& callbacks,
    EntityAnimationDispatchResult result) {
    if (!callbacks.resolve_target_motion) {
        result.action =
            EntityAnimationDispatchAction::target_motion_unresolved;
        mark_missing(result);
        return result;
    }

    const auto motion = callbacks.resolve_target_motion(state);
    if (!motion.source_carry) {
        state.animation_mode = 2U;
        result.action = EntityAnimationDispatchAction::target_motion_waiting;
        return result;
    }

    // 0x141A8-0x141AC copies the entity's current position into the
    // target-position fields before selecting the next behavior mode.
    state.target_x = state.current_x;
    state.target_y = state.current_y;
    result.target_destination_copied = true;

    if (state.transition_marker >= 0) {
        state.animation_mode = 3U;
        if (callbacks.finish_animation) {
            callbacks.finish_animation();
        } else {
            mark_missing(result);
        }
        result.action = EntityAnimationDispatchAction::target_motion_finished;
        return result;
    }

    state.animation_mode = 1U;
    if ((state.record_flags & 0x40U) != 0U) {
        if (callbacks.synchronize_behavior_spawn) {
            callbacks.synchronize_behavior_spawn();
        } else {
            mark_missing(result);
        }
        result.action =
            EntityAnimationDispatchAction::target_motion_spawn_synchronized;
        return result;
    }

    if (callbacks.configure_target_transition) {
        callbacks.configure_target_transition();
    } else {
        mark_missing(result);
    }
    result.action =
        EntityAnimationDispatchAction::target_motion_transition_configured;
    return result;
}

EntityAnimationDispatchResult enter_cleanup(
    EntityAnimationDispatchState& state,
    const EntityAnimationDispatchCallbacks& callbacks,
    EntityAnimationDispatchResult result) {
    if (state.entity_type != 0U) {
        if (!callbacks.resource_gate) {
            mark_missing(result);
            result.action =
                EntityAnimationDispatchAction::cleanup_resource_available;
            return result;
        }
        if (callbacks.resource_gate()) {
            return source_failure(
                result,
                EntityAnimationDispatchAction::cleanup_resource_blocked);
        }
        result.action = EntityAnimationDispatchAction::cleanup_resource_available;
        return result;
    }

    if ((state.entity_flags & 0x08U) != 0U && state.action_counter != 0U) {
        if (state.transition_marker < 0) {
            // The byte decrement belongs to 0x14226 itself.  The callback is
            // the audio/delay/asset boundary called by that routine.
            state.action_counter = static_cast<std::uint8_t>(
                state.action_counter - 1U);
            if (callbacks.service_action) {
                callbacks.service_action();
            } else {
                mark_missing(result);
            }
            result.action = EntityAnimationDispatchAction::action_serviced;
            return result;
        }
        result.action = EntityAnimationDispatchAction::cleanup_noop;
        return result;
    }

    state.phase = 0U;
    state.frame = static_cast<std::int8_t>(0x7FU);
    if (callbacks.update_animation_delay) {
        callbacks.update_animation_delay();
    } else {
        mark_missing(result);
    }
    if (callbacks.upload_assets) {
        callbacks.upload_assets();
    } else {
        mark_missing(result);
    }
    result.action =
        EntityAnimationDispatchAction::animation_reset_for_target_motion;
    return enter_target_motion(state, callbacks, result);
}

} // namespace

EntityAnimationDispatchResult dispatch_entity_animation_state(
    EntityAnimationDispatchState& state,
    const EntityAnimationDispatchCallbacks& callbacks) {
    EntityAnimationDispatchResult result{};
    state.animation_mode = 0U;

    if (callbacks.refresh_link) {
        callbacks.refresh_link(state.linked_target);
        result.link_refresh_called = true;
    }

    if (!state.linked_target.has_value()) {
        if ((state.entity_flags & 0x08U) == 0U) {
            result.action = EntityAnimationDispatchAction::no_linked_target;
            return result;
        }
        if (callbacks.fallback_unlinked) {
            callbacks.fallback_unlinked();
        } else {
            mark_missing(result);
        }
        result.action = EntityAnimationDispatchAction::unlinked_fallback;
        return result;
    }

    if (state.phase != 0U) {
        if (state.transition_marker >= 0) {
            return enter_cleanup(state, callbacks, result);
        }
        if (state.collision_state == 0) {
            return source_failure(
                result,
                EntityAnimationDispatchAction::
                    transition_blocked_without_collision);
        }

        if (state.frame < 0) {
            if (state.phase == 0x04U || state.phase == 0x0BU) {
                result.action = EntityAnimationDispatchAction::held_special_phase;
                return result;
            }

            if (state.phase > 0x0CU && state.phase < 0x10U) {
                if (run_resource_gate(callbacks, result)) {
                    return source_failure(
                        result,
                        EntityAnimationDispatchAction::
                            alternate_phase_resource_blocked);
                }
                if (!callbacks.alternate_phase_gate) {
                    mark_missing(result);
                    result.action =
                        EntityAnimationDispatchAction::target_motion_unresolved;
                    return result;
                }
                if (callbacks.alternate_phase_gate()) {
                    result.action =
                        EntityAnimationDispatchAction::alternate_phase_gate_blocked;
                    return result;
                }
                apply_heading(callbacks.apply_alternate_heading,
                              state.linked_target->heading, result);
                result.action =
                    EntityAnimationDispatchAction::alternate_heading_prepared;
                return result;
            }

            if (state.phase == 0U) {
                return enter_target_motion(state, callbacks, result);
            }
            if (run_resource_gate(callbacks, result)) {
                state.resource_window_marker = 0U;
                return source_failure(
                    result,
                    EntityAnimationDispatchAction::transition_resource_blocked);
            }
        } else {
            if (state.phase >= 0x0DU && state.phase < 0x10U) {
                if (run_resource_gate(callbacks, result)) {
                    return source_failure(
                        result,
                        EntityAnimationDispatchAction::
                            alternate_phase_resource_blocked);
                }
                if (!callbacks.alternate_phase_gate) {
                    mark_missing(result);
                    result.action =
                        EntityAnimationDispatchAction::target_motion_unresolved;
                    return result;
                }
                if (callbacks.alternate_phase_gate()) {
                    result.action =
                        EntityAnimationDispatchAction::alternate_phase_gate_blocked;
                    return result;
                }
                apply_heading(callbacks.apply_alternate_heading,
                              state.linked_target->heading, result);
                result.action =
                    EntityAnimationDispatchAction::alternate_heading_prepared;
                return result;
            }

            if (state.phase == 0U) {
                return enter_target_motion(state, callbacks, result);
            }
            if (run_resource_gate(callbacks, result)) {
                state.resource_window_marker = 0U;
                return source_failure(
                    result,
                    EntityAnimationDispatchAction::transition_resource_blocked);
            }
        }

        // 0x1411E only opens a spawn window when the world animation phase is
        // active, or when the entity is already carrying the route bit.
        if (!state.global_phase_active &&
            (state.route_flags & 0x10U) == 0U) {
            result.action =
                EntityAnimationDispatchAction::waiting_for_spawn_window;
            return result;
        }

        state.resource_window_marker = 1U;
        if (!callbacks.spawn_cycle) {
            mark_missing(result);
            result.action = EntityAnimationDispatchAction::spawn_cycle_cleanup;
            return result;
        }
        if (!callbacks.spawn_cycle()) {
            apply_heading(callbacks.apply_field_heading,
                          state.linked_target->heading, result);
            result.action = EntityAnimationDispatchAction::spawn_heading_applied;
            return result;
        }
        return enter_cleanup(state, callbacks, result);
    }

    return enter_target_motion(state, callbacks, result);
}

void service_entity_animation_action(
    EntityAnimationActionServiceState& state,
    const EntityAnimationActionServiceCallbacks& callbacks) {
    if (callbacks.service_audio) {
        callbacks.service_audio();
    }
    if (callbacks.update_animation_delay) {
        callbacks.update_animation_delay();
    }
    state.pending_actions = static_cast<std::uint8_t>(
        state.pending_actions - 1U);
    if (callbacks.advance_asset_meter) {
        callbacks.advance_asset_meter();
    }
}

namespace {

void mark_coordinator_boundary(
    EntityAnimationCoordinatorResult& result,
    const EntityAnimationCoordinatorBoundary boundary) {
    result.action = EntityAnimationCoordinatorAction::boundary_unresolved;
    result.unresolved_boundary = boundary;
}

bool schedule_target_continuation(
    const EntityAnimationCoordinatorCallbacks& callbacks,
    EntityAnimationCoordinatorResult& result) {
    if (!callbacks.schedule_target_continuation) {
        mark_coordinator_boundary(
            result, EntityAnimationCoordinatorBoundary::target_continuation);
        return false;
    }
    callbacks.schedule_target_continuation();
    result.action =
        EntityAnimationCoordinatorAction::special_animation_continuation_scheduled;
    return true;
}

EntityAnimationCoordinatorResult dispatch_to_motion(
    EntityAnimationCoordinatorState& state,
    const EntityAnimationCoordinatorCallbacks& callbacks,
    EntityAnimationCoordinatorResult result) {
    if (state.linked_anchor.has_value()) {
        if (!callbacks.measure_relative_heading_distance) {
            mark_coordinator_boundary(
                result,
                EntityAnimationCoordinatorBoundary::relative_heading_measurement);
            return result;
        }

        const auto measurement = callbacks.measure_relative_heading_distance(
            state.current_x, state.current_y, state.linked_anchor->x,
            state.linked_anchor->y);
        result.relative_measurement_called = true;
        if (!measurement.has_value()) {
            mark_coordinator_boundary(
                result,
                EntityAnimationCoordinatorBoundary::relative_heading_measurement);
            return result;
        }
        state.relative_heading = measurement->direction;
        state.relative_distance = measurement->distance;
    }

    if (!callbacks.dispatch_animation_state) {
        mark_coordinator_boundary(
            result, EntityAnimationCoordinatorBoundary::animation_dispatch);
        return result;
    }

    const auto dispatch = callbacks.dispatch_animation_state(state);
    result.dispatch_called = true;
    result.dispatch_action = dispatch.action;
    result.source_dispatch_failure = dispatch.source_failure_boundary;
    if (dispatch.boundary_unresolved &&
        !result.unresolved_boundary.has_value()) {
        result.unresolved_boundary =
            EntityAnimationCoordinatorBoundary::animation_dispatch;
    }

    if (dispatch.source_failure_boundary) {
        if (!callbacks.prepare_target_route) {
            mark_coordinator_boundary(
                result,
                EntityAnimationCoordinatorBoundary::active_world_target_route);
            return result;
        }
        callbacks.prepare_target_route(
            state, EntityAnimationCoordinatorTargetRouteReason::
                       animation_dispatch_failure);
        result.action =
            EntityAnimationCoordinatorAction::animation_dispatch_failure_routed;
        return result;
    }

    if (!callbacks.run_animation_motion) {
        mark_coordinator_boundary(
            result, EntityAnimationCoordinatorBoundary::animation_motion);
        return result;
    }
    callbacks.run_animation_motion(state);
    result.motion_called = true;

    // 0x13A40 preserves A1 across 0x13D0C and publishes the linked record's
    // coordinates as the next target.  This is a typed copy, not an A-register
    // lifetime trick.
    if (state.linked_anchor.has_value()) {
        state.target_x = state.linked_anchor->x;
        state.target_y = state.linked_anchor->y;
        result.linked_anchor_copied = true;
        result.action = EntityAnimationCoordinatorAction::
            linked_animation_dispatched_to_motion;
        return result;
    }

    result.action =
        EntityAnimationCoordinatorAction::animation_dispatched_to_motion;
    return result;
}

} // namespace

EntityAnimationCoordinatorResult run_entity_animation_update_coordinator(
    EntityAnimationCoordinatorState& state,
    const EntityAnimationCoordinatorCallbacks& callbacks) {
    EntityAnimationCoordinatorResult result{};

    state.timer = static_cast<std::uint16_t>(state.timer - 1U);
    if (static_cast<std::int16_t>(state.timer) >= 0) {
        result.action = EntityAnimationCoordinatorAction::waiting_for_timer;
        return result;
    }
    result.timer_expired = true;

    if (state.global_frame_lock) {
        result.action =
            EntityAnimationCoordinatorAction::suppressed_by_frame_lock;
        return result;
    }

    state.timer = 0x0014U;
    result.timer_rearmed = true;

    if (state.record_type == 0x000FU) {
        if (state.animation_elapsed > 0x003CU) {
            result.action =
                EntityAnimationCoordinatorAction::special_animation_window_closed;
            return result;
        }

        state.special_tick = static_cast<std::uint8_t>(
            state.special_tick - static_cast<std::uint8_t>(1U));
        if (static_cast<std::int8_t>(state.special_tick) >= 0) {
            schedule_target_continuation(callbacks, result);
            return result;
        }

        state.special_tick = 8U;
        if (!callbacks.select_special_link) {
            mark_coordinator_boundary(
                result,
                EntityAnimationCoordinatorBoundary::special_link_selection);
            return result;
        }
        callbacks.select_special_link(state.linked_anchor);

        if (!callbacks.update_special_heading) {
            mark_coordinator_boundary(
                result,
                EntityAnimationCoordinatorBoundary::special_heading_update);
            return result;
        }
        callbacks.update_special_heading();

        if (state.alternate_animation) {
            if (!callbacks.prepare_alternate_animation) {
                mark_coordinator_boundary(
                    result,
                    EntityAnimationCoordinatorBoundary::
                        special_animation_preparation);
                return result;
            }
            callbacks.prepare_alternate_animation();
            result.action = EntityAnimationCoordinatorAction::
                special_animation_alternate_prepared;
            return result;
        }

        if (!callbacks.prepare_primary_animation) {
            mark_coordinator_boundary(
                result,
                EntityAnimationCoordinatorBoundary::
                    special_animation_preparation);
            return result;
        }
        callbacks.prepare_primary_animation();
        result.action =
            EntityAnimationCoordinatorAction::special_animation_primary_prepared;
        return result;
    }

    if ((state.route_flags & 0x80U) != 0U) {
        if (!callbacks.prepare_fallback_target) {
            mark_coordinator_boundary(
                result, EntityAnimationCoordinatorBoundary::fallback_target);
            return result;
        }
        callbacks.prepare_fallback_target();
        result.action = EntityAnimationCoordinatorAction::fallback_target_prepared;
        return result;
    }

    if ((state.transition_flags & 0x0008U) != 0U) {
        result.action = EntityAnimationCoordinatorAction::transition_phase_held;
        return result;
    }

    if ((state.transition_flags & 0x0020U) != 0U &&
        state.world_resource_active) {
        if (!callbacks.service_boundary_transition) {
            mark_coordinator_boundary(
                result,
                EntityAnimationCoordinatorBoundary::boundary_transition);
            return result;
        }
        callbacks.service_boundary_transition();
        result.action =
            EntityAnimationCoordinatorAction::boundary_transition_serviced;
        return result;
    }

    if (state.collision_state == 0 &&
        (state.transition_flags & 0x0004U) != 0U) {
        if (!callbacks.clear_effect_slot) {
            mark_coordinator_boundary(
                result,
                EntityAnimationCoordinatorBoundary::effect_slot_cleanup);
            return result;
        }
        callbacks.clear_effect_slot();
        result.action = EntityAnimationCoordinatorAction::effect_slot_cleared;
        return result;
    }

    if ((state.route_flags & 0x40U) != 0U) {
        if (!callbacks.service_boundary_transition) {
            mark_coordinator_boundary(
                result,
                EntityAnimationCoordinatorBoundary::boundary_transition);
            return result;
        }
        callbacks.service_boundary_transition();
        result.action =
            EntityAnimationCoordinatorAction::boundary_transition_serviced;
        return result;
    }

    if (state.collision_state == 0) {
        if (!state.world_resource_active) {
            if (!callbacks.prepare_inactive_world_target_route) {
                mark_coordinator_boundary(
                    result,
                    EntityAnimationCoordinatorBoundary::
                        inactive_world_target_route);
                return result;
            }
            callbacks.prepare_inactive_world_target_route();
            result.action = EntityAnimationCoordinatorAction::
                inactive_world_target_prepared;
            return result;
        }

        // 0x13996-0x139B0 is the stationary entity-0x28 exception.  Its
        // target continuation is the same 0x13E40 -> 0x145A2 tail used by the
        // type-0x0F path.
        if (state.world_phase_active &&
            (state.entity_flags & 0x08U) == 0U &&
            state.entity_kind == 0x28U) {
            schedule_target_continuation(callbacks, result);
            return result;
        }

        if (!callbacks.prepare_target_route) {
            mark_coordinator_boundary(
                result,
                EntityAnimationCoordinatorBoundary::active_world_target_route);
            return result;
        }
        callbacks.prepare_target_route(
            state,
            EntityAnimationCoordinatorTargetRouteReason::active_world_idle);
        result.action =
            EntityAnimationCoordinatorAction::active_world_target_prepared;
        return result;
    }

    if ((state.transition_flags & 0x0040U) != 0U) {
        if (!callbacks.schedule_world_activity_timeout) {
            mark_coordinator_boundary(
                result,
                EntityAnimationCoordinatorBoundary::boundary_transition);
            return result;
        }
        callbacks.schedule_world_activity_timeout();
        result.activity_timeout_called = true;
    }

    // 0x13EF4 is entered only when the world resource is inactive.  The
    // resource conversion has no meaningful return dependency here, so its
    // optional callback is observed and the source proceeds to 0x13D8.
    if (!state.world_resource_active) {
        if (callbacks.run_resource_gate) {
            callbacks.run_resource_gate();
            result.resource_gate_called = true;
        } else {
            mark_coordinator_boundary(
                result, EntityAnimationCoordinatorBoundary::resource_gate);
        }
    }

    return dispatch_to_motion(state, callbacks, result);
}

} // namespace shadowrun::ghidra_conversion
