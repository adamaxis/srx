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

} // namespace shadowrun::ghidra_conversion
