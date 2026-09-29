#include "shadowrun_ghidra/entities/entity_behavior_action_service_conversion.hpp"

#include <cassert>

using namespace shadowrun::ghidra_conversion;

int main() {
    EntityBehaviorFrameRecord redirected_record{};
    redirected_record.transition_gate = -1;
    redirected_record.action_heading_source = 0x9000U;
    EntityBehaviorActionWorldState redirected_world{};
    redirected_world.movement_transition_active = true;
    const auto redirected = advance_entity_behavior_action(
        redirected_record, redirected_world, {});
    assert(redirected.kind ==
           EntityBehaviorActionResultKind::movement_transition_redirected);
    assert(redirected_record.current_heading == 0x1000U &&
           redirected_record.target_heading == 0x1000U &&
           redirected_record.continuation_kind ==
               EntityBehaviorContinuationKind::movement_route);

    EntityBehaviorFrameRecord delayed_record{};
    delayed_record.transition_gate = -1;
    EntityBehaviorActionWorldState delayed_world{};
    delayed_world.activity_lock = true;
    delayed_world.activity_countdown = 2U;
    const auto waiting = advance_entity_behavior_action(
        delayed_record, delayed_world, {});
    assert(waiting.kind ==
               EntityBehaviorActionResultKind::activity_countdown_waiting &&
           delayed_world.activity_countdown == 1U &&
           delayed_record.continuation_kind ==
               EntityBehaviorContinuationKind::collision_recheck);
    delayed_world.activity_countdown = 0U;
    const auto expired = advance_entity_behavior_action(
        delayed_record, delayed_world, {});
    assert(expired.kind ==
               EntityBehaviorActionResultKind::activity_countdown_expired &&
           delayed_record.target_heading == 0U &&
           delayed_record.continuation_kind ==
               EntityBehaviorContinuationKind::movement_route);

    EntityBehaviorFrameRecord heading_record{};
    heading_record.transition_gate = -1;
    heading_record.auxiliary_marker = 3U;
    heading_record.target_marker = -1;
    EntityBehaviorActionWorldState heading_world{};
    heading_world.controller_action = 3;
    int heading_type_calls = 0;
    EntityBehaviorActionCallbacks heading_callbacks{};
    heading_callbacks.update_entity_target_heading =
        [&](EntityBehaviorFrameRecord&) { ++heading_type_calls; };
    const auto heading_result = advance_entity_behavior_action(
        heading_record, heading_world, heading_callbacks);
    assert(heading_result.kind ==
               EntityBehaviorActionResultKind::controller_heading_prepared &&
           heading_record.target_heading == 3U &&
           heading_record.auxiliary_marker == 0U &&
           heading_type_calls == 0 &&
           heading_record.continuation_kind ==
               EntityBehaviorContinuationKind::movement_route);

    EntityBehaviorFrameRecord delay_record{};
    delay_record.transition_gate = -1;
    delay_record.auxiliary_marker = 4U;
    EntityBehaviorActionWorldState delay_world{};
    int idle_delay_mode = -1;
    EntityBehaviorActionCallbacks delay_callbacks{};
    delay_callbacks.update_entity_animation_delay =
        [&](EntityBehaviorFrameRecord&, const std::uint16_t mode) {
            idle_delay_mode = static_cast<int>(mode);
        };
    const auto delay_result = advance_entity_behavior_action(
        delay_record, delay_world, delay_callbacks);
    assert(delay_result.kind ==
               EntityBehaviorActionResultKind::collision_recheck_prepared &&
           idle_delay_mode == 1 && delay_record.auxiliary_marker == 5U &&
           delay_record.continuation_kind ==
               EntityBehaviorContinuationKind::collision_recheck);

    EntityBehaviorFrameRecord target_record{};
    target_record.transition_gate = -1;
    target_record.target_marker = -1;
    EntityBehaviorActionWorldState target_world{};
    target_world.controller_events = 0x0040U;
    target_world.controller_action = 1;
    EntityBehaviorActionCallbacks target_callbacks{};
    int target_heading_calls = 0;
    int primary_animation_calls = 0;
    target_callbacks.update_entity_target_heading =
        [&](EntityBehaviorFrameRecord&) { ++target_heading_calls; };
    target_callbacks.update_entity_animation_delay =
        [&](EntityBehaviorFrameRecord&, std::uint16_t) {};
    target_callbacks.prepare_delayed_animation_primary =
        [&](EntityBehaviorFrameRecord&) { ++primary_animation_calls; };
    const auto target_result = advance_entity_behavior_action(
        target_record, target_world, target_callbacks);
    assert(target_result.kind ==
               EntityBehaviorActionResultKind::target_animation_prepared &&
           target_heading_calls == 1 && primary_animation_calls == 1);

    EntityBehaviorFrameRecord effect_record{};
    effect_record.transition_gate = -1;
    effect_record.source_type = 0x0DU;
    EntityBehaviorActionWorldState effect_world{};
    effect_world.controller_events = 0x0040U;
    effect_world.controller_action = 1;
    EntityBehaviorActionCallbacks effect_callbacks{};
    int alternate_heading_calls = 0;
    effect_callbacks.spawn_entity_effect_record =
        [&](EntityBehaviorFrameRecord&) {
            return EntityEffectSpawnDispatchResult{false, 0x1234U};
        };
    effect_callbacks.set_entity_heading_with_alternate_animation =
        [&](EntityBehaviorFrameRecord&, const std::uint16_t heading) {
            ++alternate_heading_calls;
            assert(heading == 0x1234U);
        };
    const auto effect_result = advance_entity_behavior_action(
        effect_record, effect_world, effect_callbacks);
    assert(effect_result.kind ==
               EntityBehaviorActionResultKind::effect_heading_prepared &&
           alternate_heading_calls == 1);

    EntityBehaviorFrameRecord spawn_record{};
    spawn_record.transition_gate = -1;
    spawn_record.source_type = 1U;
    EntityBehaviorActionWorldState spawn_world{};
    spawn_world.controller_events = 0x0040U;
    spawn_world.controller_action = 1;
    EntityBehaviorActionCallbacks spawn_callbacks{};
    int field_heading_calls = 0;
    spawn_callbacks.process_entity_spawn_cycle =
        [&](EntityBehaviorFrameRecord&) {
            return EntitySpawnCycleDispatchResult{false, 0x4321U};
        };
    spawn_callbacks.set_entity_heading_with_field_animation =
        [&](EntityBehaviorFrameRecord&, const std::uint16_t heading) {
            ++field_heading_calls;
            assert(heading == 0x4321U);
        };
    spawn_callbacks.dispatch_player_action_if_ready =
        [&](EntityBehaviorFrameRecord&) { return false; };
    spawn_callbacks.service_audio_if_enabled = [&](std::uint16_t) {};
    spawn_callbacks.update_entity_animation_delay =
        [&](EntityBehaviorFrameRecord&, std::uint16_t) {};
    const auto spawn_result = advance_entity_behavior_action(
        spawn_record, spawn_world, spawn_callbacks);
    assert(spawn_result.kind ==
               EntityBehaviorActionResultKind::spawn_cycle_heading_prepared &&
           field_heading_calls == 1);

    EntityBehaviorFrameRecord action_record{};
    action_record.transition_gate = -1;
    action_record.source_type = 1U;
    EntityBehaviorActionWorldState action_world{};
    action_world.controller_events = 0x0040U;
    action_world.controller_action = 4;
    EntityBehaviorActionCallbacks action_callbacks{};
    int audio_event = 0;
    int delay_mode = -1;
    action_callbacks.process_entity_spawn_cycle =
        [&](EntityBehaviorFrameRecord&) {
            return EntitySpawnCycleDispatchResult{true, 0};
        };
    action_callbacks.dispatch_player_action_if_ready =
        [&](EntityBehaviorFrameRecord&) { return true; };
    action_callbacks.service_audio_if_enabled =
        [&](const std::uint16_t event) { audio_event = event; };
    action_callbacks.update_entity_animation_delay =
        [&](EntityBehaviorFrameRecord&, const std::uint16_t mode) {
            delay_mode = static_cast<int>(mode);
        };
    action_callbacks.set_entity_heading_with_field_animation =
        [&](EntityBehaviorFrameRecord&, std::uint16_t) {};
    const auto action_result = advance_entity_behavior_action(
        action_record, action_world, action_callbacks);
    assert(action_result.kind ==
               EntityBehaviorActionResultKind::controller_heading_prepared &&
           action_result.action_dispatched && audio_event == 0x4B &&
           delay_mode == 0 && action_record.target_heading == 4 &&
           action_record.continuation_kind ==
               EntityBehaviorContinuationKind::movement_route);

    EntityBehaviorFrameRecord missing_record{};
    missing_record.transition_gate = -1;
    missing_record.source_type = 1U;
    EntityBehaviorActionWorldState missing_world{};
    missing_world.controller_events = 0x0040U;
    const auto unresolved = advance_entity_behavior_action(
        missing_record, missing_world, {});
    assert(unresolved.kind ==
           EntityBehaviorActionResultKind::callback_unresolved);
    return 0;
}
