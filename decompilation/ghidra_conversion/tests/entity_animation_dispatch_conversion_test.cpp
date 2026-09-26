#include "shadowrun_ghidra/entity_animation_dispatch_conversion.hpp"

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

    return 0;
}
