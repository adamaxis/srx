#include "shadowrun_ghidra/entities/entity_animation_dispatch_conversion.hpp"

#include <vector>

namespace {

using namespace shadowrun::ghidra_conversion;

EntityAnimationDispatchTarget target(const std::uint16_t heading = 0x4000U) {
    return {7U, heading};
}

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityAnimationDispatchState no_target{};
    const auto no_target_result =
        dispatch_entity_animation_state(no_target, {});
    if (!expect(no_target_result.action ==
                EntityAnimationDispatchAction::no_linked_target)) {
        return 1;
    }

    EntityAnimationDispatchState fallback{};
    fallback.entity_flags = 0x08U;
    bool fallback_called = false;
    EntityAnimationDispatchCallbacks fallback_callbacks{};
    fallback_callbacks.fallback_unlinked =
        [&fallback_called]() { fallback_called = true; };
    const auto fallback_result =
        dispatch_entity_animation_state(fallback, fallback_callbacks);
    if (!expect(fallback_result.action ==
                    EntityAnimationDispatchAction::unlinked_fallback &&
                fallback_called)) {
        return 2;
    }

    EntityAnimationDispatchState blocked{};
    blocked.linked_target = target();
    blocked.phase = 1U;
    blocked.transition_marker = -1;
    const auto blocked_result =
        dispatch_entity_animation_state(blocked, {});
    if (!expect(blocked_result.action ==
                    EntityAnimationDispatchAction::
                        transition_blocked_without_collision &&
                blocked_result.source_failure_boundary)) {
        return 3;
    }

    EntityAnimationDispatchState held{};
    held.linked_target = target();
    held.phase = 4U;
    held.frame = -1;
    held.transition_marker = -1;
    held.collision_state = 1;
    const auto held_result = dispatch_entity_animation_state(held, {});
    if (!expect(held_result.action ==
                EntityAnimationDispatchAction::held_special_phase)) {
        return 4;
    }

    EntityAnimationDispatchState alternate_blocked{};
    alternate_blocked.linked_target = target();
    alternate_blocked.phase = 0x0DU;
    alternate_blocked.frame = -1;
    alternate_blocked.transition_marker = -1;
    alternate_blocked.collision_state = 1;
    EntityAnimationDispatchCallbacks alternate_blocked_callbacks{};
    alternate_blocked_callbacks.resource_gate = []() { return true; };
    const auto alternate_blocked_result = dispatch_entity_animation_state(
        alternate_blocked, alternate_blocked_callbacks);
    if (!expect(alternate_blocked_result.action ==
                    EntityAnimationDispatchAction::
                        alternate_phase_resource_blocked &&
                alternate_blocked_result.source_failure_boundary)) {
        return 5;
    }

    EntityAnimationDispatchState alternate_ready{};
    alternate_ready.linked_target = target(0x1234U);
    alternate_ready.phase = 0x0DU;
    alternate_ready.frame = 0;
    alternate_ready.transition_marker = -1;
    alternate_ready.collision_state = 1;
    bool alternate_heading_called = false;
    EntityAnimationDispatchCallbacks alternate_ready_callbacks{};
    alternate_ready_callbacks.resource_gate = []() { return false; };
    alternate_ready_callbacks.alternate_phase_gate = []() { return false; };
    alternate_ready_callbacks.apply_alternate_heading =
        [&alternate_heading_called](const std::uint16_t heading) {
            alternate_heading_called = heading == 0x1234U;
        };
    const auto alternate_ready_result = dispatch_entity_animation_state(
        alternate_ready, alternate_ready_callbacks);
    if (!expect(alternate_ready_result.action ==
                    EntityAnimationDispatchAction::alternate_heading_prepared &&
                alternate_heading_called &&
                alternate_ready_result.applied_heading ==
                    std::optional<std::uint16_t>{0x1234U})) {
        return 6;
    }

    EntityAnimationDispatchState waiting{};
    waiting.linked_target = target();
    waiting.phase = 1U;
    waiting.frame = 0;
    waiting.transition_marker = -1;
    waiting.collision_state = 1;
    EntityAnimationDispatchCallbacks waiting_callbacks{};
    waiting_callbacks.resource_gate = []() { return false; };
    const auto waiting_result =
        dispatch_entity_animation_state(waiting, waiting_callbacks);
    if (!expect(waiting_result.action ==
                EntityAnimationDispatchAction::waiting_for_spawn_window)) {
        return 7;
    }

    EntityAnimationDispatchState spawn_heading{};
    spawn_heading.linked_target = target(0x5678U);
    spawn_heading.phase = 1U;
    spawn_heading.frame = 0;
    spawn_heading.transition_marker = -1;
    spawn_heading.collision_state = 1;
    spawn_heading.global_phase_active = true;
    bool field_heading_called = false;
    EntityAnimationDispatchCallbacks spawn_heading_callbacks{};
    spawn_heading_callbacks.resource_gate = []() { return false; };
    spawn_heading_callbacks.spawn_cycle = []() { return false; };
    spawn_heading_callbacks.apply_field_heading =
        [&field_heading_called](const std::uint16_t heading) {
            field_heading_called = heading == 0x5678U;
        };
    const auto spawn_heading_result = dispatch_entity_animation_state(
        spawn_heading, spawn_heading_callbacks);
    if (!expect(spawn_heading_result.action ==
                    EntityAnimationDispatchAction::spawn_heading_applied &&
                field_heading_called && spawn_heading.resource_window_marker ==
                    1U)) {
        return 8;
    }

    EntityAnimationDispatchState serviced{};
    serviced.linked_target = target();
    serviced.phase = 1U;
    serviced.frame = 0;
    serviced.transition_marker = -1;
    serviced.collision_state = 1;
    serviced.global_phase_active = true;
    serviced.entity_flags = 0x08U;
    serviced.action_counter = 2U;
    EntityAnimationDispatchCallbacks serviced_callbacks{};
    serviced_callbacks.resource_gate = []() { return false; };
    serviced_callbacks.spawn_cycle = []() { return true; };
    bool service_called = false;
    serviced_callbacks.service_action =
        [&service_called]() { service_called = true; };
    const auto serviced_result =
        dispatch_entity_animation_state(serviced, serviced_callbacks);
    if (!expect(serviced_result.action ==
                    EntityAnimationDispatchAction::action_serviced &&
                service_called && serviced.action_counter == 1U)) {
        return 9;
    }

    EntityAnimationDispatchState target_wait{};
    target_wait.linked_target = target();
    target_wait.current_x = 0x20U;
    target_wait.current_y = 0x30U;
    EntityAnimationDispatchCallbacks target_wait_callbacks{};
    target_wait_callbacks.resolve_target_motion =
        [](EntityAnimationDispatchState&) {
            return EntityTargetMotionResult{
                EntityTargetMotionAction::target_out_of_range, false};
        };
    const auto target_wait_result =
        dispatch_entity_animation_state(target_wait, target_wait_callbacks);
    if (!expect(target_wait_result.action ==
                    EntityAnimationDispatchAction::target_motion_waiting &&
                target_wait.animation_mode == 2U)) {
        return 10;
    }

    EntityAnimationDispatchState finished{};
    finished.linked_target = target();
    finished.current_x = 0x20U;
    finished.current_y = 0x30U;
    finished.transition_marker = 0;
    bool finished_called = false;
    EntityAnimationDispatchCallbacks finished_callbacks{};
    finished_callbacks.resolve_target_motion =
        [](EntityAnimationDispatchState&) {
            return EntityTargetMotionResult{
                EntityTargetMotionAction::target_heading_applied, true};
        };
    finished_callbacks.finish_animation =
        [&finished_called]() { finished_called = true; };
    const auto finished_result =
        dispatch_entity_animation_state(finished, finished_callbacks);
    if (!expect(finished_result.action ==
                    EntityAnimationDispatchAction::target_motion_finished &&
                finished.animation_mode == 3U && finished_called &&
                finished.target_x == 0x20U && finished.target_y == 0x30U)) {
        return 11;
    }

    EntityAnimationDispatchState synchronized{};
    synchronized.linked_target = target();
    synchronized.record_flags = 0x40U;
    synchronized.transition_marker = -1;
    bool synchronize_called = false;
    EntityAnimationDispatchCallbacks synchronized_callbacks{};
    synchronized_callbacks.resolve_target_motion =
        [](EntityAnimationDispatchState&) {
            return EntityTargetMotionResult{
                EntityTargetMotionAction::target_heading_applied, true};
        };
    synchronized_callbacks.synchronize_behavior_spawn =
        [&synchronize_called]() { synchronize_called = true; };
    const auto synchronized_result = dispatch_entity_animation_state(
        synchronized, synchronized_callbacks);
    if (!expect(synchronized_result.action == EntityAnimationDispatchAction::
                    target_motion_spawn_synchronized &&
                synchronized.animation_mode == 1U && synchronize_called)) {
        return 12;
    }

    EntityAnimationDispatchState configured{};
    configured.linked_target = target();
    configured.transition_marker = -1;
    bool configured_called = false;
    EntityAnimationDispatchCallbacks configured_callbacks{};
    configured_callbacks.resolve_target_motion =
        [](EntityAnimationDispatchState&) {
            return EntityTargetMotionResult{
                EntityTargetMotionAction::target_heading_applied, true};
        };
    configured_callbacks.configure_target_transition =
        [&configured_called]() { configured_called = true; };
    const auto configured_result =
        dispatch_entity_animation_state(configured, configured_callbacks);
    if (!expect(configured_result.action == EntityAnimationDispatchAction::
                    target_motion_transition_configured &&
                configured.animation_mode == 1U && configured_called)) {
        return 13;
    }

    EntityAnimationActionServiceState action_state{2U};
    std::vector<char> action_order;
    service_entity_animation_action(
        action_state,
        {[&action_order]() { action_order.push_back('a'); },
         [&action_order]() { action_order.push_back('d'); },
         [&action_order]() { action_order.push_back('m'); }});
    if (!expect(action_state.pending_actions == 1U &&
                action_order == std::vector<char>{'a', 'd', 'm'})) {
        return 14;
    }

    EntityAnimationCoordinatorState coordinator_waiting{};
    coordinator_waiting.timer = 2U;
    const auto coordinator_wait_result =
        run_entity_animation_update_coordinator(coordinator_waiting, {});
    if (!expect(coordinator_wait_result.action ==
                    EntityAnimationCoordinatorAction::waiting_for_timer &&
                coordinator_waiting.timer == 1U)) {
        return 15;
    }

    EntityAnimationCoordinatorState coordinator_locked{};
    coordinator_locked.global_frame_lock = true;
    const auto coordinator_locked_result =
        run_entity_animation_update_coordinator(coordinator_locked, {});
    if (!expect(coordinator_locked_result.action ==
                    EntityAnimationCoordinatorAction::suppressed_by_frame_lock &&
                coordinator_locked_result.timer_expired &&
                !coordinator_locked_result.timer_rearmed)) {
        return 16;
    }

    EntityAnimationCoordinatorState special_waiting{};
    special_waiting.record_type = 0x0FU;
    special_waiting.animation_elapsed = 0x20U;
    special_waiting.special_tick = 2U;
    bool continuation_called = false;
    EntityAnimationCoordinatorCallbacks special_waiting_callbacks{};
    special_waiting_callbacks.schedule_target_continuation =
        [&continuation_called]() { continuation_called = true; };
    const auto special_waiting_result = run_entity_animation_update_coordinator(
        special_waiting, special_waiting_callbacks);
    if (!expect(
            special_waiting_result.action ==
                EntityAnimationCoordinatorAction::
                    special_animation_continuation_scheduled &&
            continuation_called && special_waiting.timer == 0x14U &&
            special_waiting.special_tick == 1U)) {
        return 17;
    }

    EntityAnimationCoordinatorState special_primary{};
    special_primary.record_type = 0x0FU;
    special_primary.animation_elapsed = 0x20U;
    std::vector<char> special_order;
    EntityAnimationCoordinatorCallbacks special_primary_callbacks{};
    special_primary_callbacks.select_special_link =
        [&special_order](std::optional<EntityAnimationCoordinatorAnchor>& link) {
            special_order.push_back('s');
            link = EntityAnimationCoordinatorAnchor{3U, 0x1000U, 0x1100U,
                                                     0x4000U};
        };
    special_primary_callbacks.update_special_heading =
        [&special_order]() { special_order.push_back('h'); };
    special_primary_callbacks.prepare_primary_animation =
        [&special_order]() { special_order.push_back('p'); };
    const auto special_primary_result = run_entity_animation_update_coordinator(
        special_primary, special_primary_callbacks);
    if (!expect(
            special_primary_result.action ==
                EntityAnimationCoordinatorAction::
                    special_animation_primary_prepared &&
            special_order == std::vector<char>{'s', 'h', 'p'} &&
            special_primary.special_tick == 8U &&
            special_primary.linked_anchor.has_value())) {
        return 18;
    }

    EntityAnimationCoordinatorState fallback_coordinator{};
    fallback_coordinator.route_flags = 0x80U;
    bool fallback_target_called = false;
    EntityAnimationCoordinatorCallbacks fallback_coordinator_callbacks{};
    fallback_coordinator_callbacks.prepare_fallback_target =
        [&fallback_target_called]() { fallback_target_called = true; };
    const auto fallback_coordinator_result =
        run_entity_animation_update_coordinator(
            fallback_coordinator, fallback_coordinator_callbacks);
    if (!expect(
            fallback_coordinator_result.action ==
                EntityAnimationCoordinatorAction::fallback_target_prepared &&
            fallback_target_called)) {
        return 19;
    }

    EntityAnimationCoordinatorState boundary_coordinator{};
    boundary_coordinator.transition_flags = 0x20U;
    boundary_coordinator.world_resource_active = true;
    bool boundary_called = false;
    EntityAnimationCoordinatorCallbacks boundary_callbacks{};
    boundary_callbacks.service_boundary_transition =
        [&boundary_called]() { boundary_called = true; };
    const auto boundary_result = run_entity_animation_update_coordinator(
        boundary_coordinator, boundary_callbacks);
    if (!expect(
            boundary_result.action ==
                EntityAnimationCoordinatorAction::
                    boundary_transition_serviced &&
            boundary_called)) {
        return 20;
    }

    EntityAnimationCoordinatorState effect_coordinator{};
    effect_coordinator.collision_state = 0;
    effect_coordinator.transition_flags = 0x04U;
    bool effect_clear_called = false;
    EntityAnimationCoordinatorCallbacks effect_callbacks{};
    effect_callbacks.clear_effect_slot =
        [&effect_clear_called]() { effect_clear_called = true; };
    const auto effect_result = run_entity_animation_update_coordinator(
        effect_coordinator, effect_callbacks);
    if (!expect(effect_result.action ==
                    EntityAnimationCoordinatorAction::effect_slot_cleared &&
                effect_clear_called)) {
        return 21;
    }

    EntityAnimationCoordinatorState linked_coordinator{};
    linked_coordinator.collision_state = 1;
    linked_coordinator.world_resource_active = true;
    linked_coordinator.current_x = 0x0100U;
    linked_coordinator.current_y = 0x0200U;
    linked_coordinator.linked_anchor =
        EntityAnimationCoordinatorAnchor{9U, 0x0300U, 0x0400U, 0x5000U};
    bool motion_called = false;
    EntityAnimationCoordinatorCallbacks linked_callbacks{};
    linked_callbacks.measure_relative_heading_distance =
        [](const std::uint16_t current_x,
           const std::uint16_t current_y,
           const std::uint16_t linked_x,
           const std::uint16_t linked_y)
        -> std::optional<EntityAnimationHeadingDistance> {
        if (current_x != 0x0100U || current_y != 0x0200U ||
            linked_x != 0x0300U || linked_y != 0x0400U) {
            return std::nullopt;
        }
        return EntityAnimationHeadingDistance{0x5000U, 0x0123U};
    };
    linked_callbacks.dispatch_animation_state =
        [](EntityAnimationCoordinatorState& state) {
            if (state.relative_heading != 0x5000U ||
                state.relative_distance != 0x0123U) {
                return EntityAnimationDispatchResult{
                    EntityAnimationDispatchAction::target_motion_unresolved,
                    false, true, true, false, false, std::nullopt};
            }
            return EntityAnimationDispatchResult{
                EntityAnimationDispatchAction::target_motion_waiting,
                false, false, false, false, false, std::nullopt};
        };
    linked_callbacks.run_animation_motion =
        [&motion_called](EntityAnimationCoordinatorState&) {
            motion_called = true;
        };
    const auto linked_result = run_entity_animation_update_coordinator(
        linked_coordinator, linked_callbacks);
    if (!expect(
            linked_result.action == EntityAnimationCoordinatorAction::
                linked_animation_dispatched_to_motion &&
            linked_result.relative_measurement_called &&
            linked_result.dispatch_called && linked_result.motion_called &&
            linked_result.linked_anchor_copied && motion_called &&
            linked_coordinator.target_x == 0x0300U &&
            linked_coordinator.target_y == 0x0400U)) {
        return 22;
    }

    EntityAnimationCoordinatorState dispatch_failure{};
    dispatch_failure.collision_state = 1;
    dispatch_failure.world_resource_active = true;
    EntityAnimationCoordinatorTargetRouteReason failure_reason =
        EntityAnimationCoordinatorTargetRouteReason::active_world_idle;
    EntityAnimationCoordinatorCallbacks failure_callbacks{};
    failure_callbacks.dispatch_animation_state =
        [](EntityAnimationCoordinatorState&) {
            return EntityAnimationDispatchResult{
                EntityAnimationDispatchAction::cleanup_resource_blocked,
                true, true, false, false, false, std::nullopt};
        };
    failure_callbacks.prepare_target_route =
        [&failure_reason](EntityAnimationCoordinatorState&,
                          const EntityAnimationCoordinatorTargetRouteReason reason) {
            failure_reason = reason;
        };
    const auto failure_result = run_entity_animation_update_coordinator(
        dispatch_failure, failure_callbacks);
    if (!expect(
            failure_result.action == EntityAnimationCoordinatorAction::
                animation_dispatch_failure_routed &&
            failure_result.source_dispatch_failure &&
            failure_reason == EntityAnimationCoordinatorTargetRouteReason::
                                  animation_dispatch_failure)) {
        return 23;
    }

    return 0;
}
