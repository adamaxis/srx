// This test uses assert as its check mechanism; keep it active in Release too.
#ifdef NDEBUG
#undef NDEBUG
#endif

#include "shadowrun_ghidra/entities/movement_mode_transition_conversion.hpp"

#include <cassert>

using namespace shadowrun::ghidra_conversion;

namespace {

EntityBehaviorFrameRecord make_record() {
    EntityBehaviorFrameRecord record{};
    record.behavior_flags = 0x02U;
    return record;
}

EntityMovementModeTransitionState make_state() {
    EntityMovementModeTransitionState state{};
    state.directional_probe_signs = {-1, -2, 1, 2};
    return state;
}

EntityMovementModeTransitionState make_state(
    const std::uint16_t terrain_collision_code) {
    auto state = make_state();
    state.terrain_collision_code = terrain_collision_code;
    return state;
}

FieldMapEventPolicyDecision make_script_decision(
    const FieldMapCollisionEventInput& input,
    const FieldMapEventAction action,
    const std::optional<FieldCollisionEventCode> event_code = std::nullopt,
    const std::optional<std::uint16_t> raw_collision_code = std::nullopt,
    const bool mode_event_code_published = false,
    const bool projected_terrain_event_published = false,
    const bool rewrite_direction_probe = false,
    const std::int8_t direction_probe_value = 0,
    const bool rewrite_quantized_delta = false,
    const std::uint16_t quantized_delta = 0U) {
    FieldMapEventPolicyDecision decision{};
    decision.action = action;
    decision.source_event_code =
        field_collision_event_code(input.raw_collision_code);
    decision.event_code = event_code.value_or(decision.source_event_code);
    decision.raw_collision_code =
        raw_collision_code.value_or(input.raw_collision_code);
    decision.mode_event_code_published = mode_event_code_published;
    decision.projected_terrain_event_published =
        projected_terrain_event_published;
    decision.rewrite_direction_probe = rewrite_direction_probe;
    decision.direction_probe_value = direction_probe_value;
    decision.rewrite_quantized_delta = rewrite_quantized_delta;
    decision.quantized_delta = quantized_delta;
    return decision;
}

EntityMovementModeTransitionCallbacks make_projection_callbacks(
    std::int16_t& quantized_value,
    int& projection_preparations) {
    EntityMovementModeTransitionCallbacks callbacks{};
    callbacks.build_collision_probe_points =
        [&](EntityBehaviorFrameRecord&) {
            ++projection_preparations;
            return MovementProjectionPreparation{3U};
        };
    callbacks.quantize_field_delta =
        [&](const std::int16_t, const std::uint16_t) {
            return quantized_value;
        };
    callbacks.dispatch_field_map_event =
        [](const FieldMapCollisionEventInput& input)
            -> std::optional<FieldMapEventPolicyDecision> {
            return make_script_decision(
                input, FieldMapEventAction::project_world_interaction);
        };
    return callbacks;
}

} // namespace

int main() {
    {
        auto record = make_record();
        auto state = make_state(0x001FU);
        state.world_interaction_lock = true;
        const auto result = advance_entity_movement_and_mode_transition(
            record, 0x200U, 0x400U, state, {});
        assert(result.kind ==
               EntityMovementModeTransitionResultKind::already_locked);
    }

    {
        auto record = make_record();
        record.route_flags = 0x20U;
        auto state = make_state();
        const auto result = advance_entity_movement_and_mode_transition(
            record, 0x200U, 0x400U, state, {});
        assert(result.kind ==
               EntityMovementModeTransitionResultKind::already_pending);
    }

    {
        auto record = make_record();
        record.behavior_flags = 0;
        auto state = make_state();
        const auto result = advance_entity_movement_and_mode_transition(
            record, 0x200U, 0x400U, state, {});
        assert(result.kind == EntityMovementModeTransitionResultKind::
                                  released_without_behavior &&
               result.record_released && (record.route_flags & 0x21U) == 1U);
    }

    {
        auto record = make_record();
        auto state = make_state();
        state.interaction_gate = 1U;
        const auto result = advance_entity_movement_and_mode_transition(
            record, 0x200U, 0x400U, state, {});
        assert(result.kind == EntityMovementModeTransitionResultKind::
                                  released_by_scene_gate &&
               result.record_released);
    }

    {
        auto record = make_record();
        auto state = make_state();
        state.directional_probe_signs = {1, 2, 3, -1};
        const auto result = advance_entity_movement_and_mode_transition(
            record, 0x200U, 0x400U, state, {});
        assert(result.kind == EntityMovementModeTransitionResultKind::
                                  released_by_probe_count &&
               result.negative_probe_count == 1U);
    }

    std::int16_t quantized_value = 0x0004U;
    int projection_preparations = 0;
    auto callbacks = make_projection_callbacks(
        quantized_value, projection_preparations);
    MovementModeTransitionContinuation last_continuation =
        MovementModeTransitionContinuation::none;
    callbacks.begin_world_activity_if_idle =
        [&](const MovementModeTransitionContinuation continuation) {
            last_continuation = continuation;
            return true;
        };
    callbacks.begin_world_activity =
        [&](const MovementModeTransitionContinuation continuation) {
            last_continuation = continuation;
        };

    {
        auto record = make_record();
        auto state = make_state(0x001FU);
        state.mode_event_code = 0x12U;
        callbacks.dispatch_field_map_event = [](
            const FieldMapCollisionEventInput& input)
            -> std::optional<FieldMapEventPolicyDecision> {
            return make_script_decision(
                input, FieldMapEventAction::begin_event_31_activity);
        };
        const auto result = advance_entity_movement_and_mode_transition(
            record, 0x1234U, 0x5678U, state, callbacks);
        assert(result.kind ==
                   EntityMovementModeTransitionResultKind::
                       world_activity_started &&
               result.continuation ==
                   MovementModeTransitionContinuation::event_31_world_activity &&
               state.activity_selection == 0U &&
               !result.mode_event_code_published &&
               state.mode_event_code == 0x12U &&
               record.action_heading_source == 0xC000U &&
               (record.route_flags & 0x21U) == 1U);
    }

    {
        quantized_value = 0x001DU;
        auto record = make_record();
        auto state = make_state(0x001DU);
        state.activity_sequence_active = true;
        callbacks.dispatch_field_map_event = [](
            const FieldMapCollisionEventInput& input)
            -> std::optional<FieldMapEventPolicyDecision> {
            return make_script_decision(
                input, FieldMapEventAction::skip_world_interaction);
        };
        const auto result = advance_entity_movement_and_mode_transition(
            record, 0x200U, 0x400U, state, callbacks);
        assert(result.kind ==
                   EntityMovementModeTransitionResultKind::projection_skipped &&
               result.record_released && last_continuation ==
                   MovementModeTransitionContinuation::event_31_world_activity);
    }

    {
        auto record = make_record();
        auto state = make_state(0x001DU);
        callbacks.dispatch_field_map_event = [](
            const FieldMapCollisionEventInput& input)
            -> std::optional<FieldMapEventPolicyDecision> {
            return make_script_decision(
                input, FieldMapEventAction::begin_event_29_activity,
                FieldCollisionEventCode::event_29);
        };
        callbacks.begin_world_activity_if_idle =
            [](const MovementModeTransitionContinuation) { return false; };
        const auto result = advance_entity_movement_and_mode_transition(
            record, 0x200U, 0x400U, state, callbacks);
        assert(result.kind == EntityMovementModeTransitionResultKind::
                                  world_activity_already_running &&
               result.continuation ==
                   MovementModeTransitionContinuation::event_29_world_activity &&
               result.event_code == FieldCollisionEventCode::event_29 &&
               result.record_released && (record.route_flags & 0x21U) == 1U);
        callbacks.begin_world_activity_if_idle =
            [&](const MovementModeTransitionContinuation continuation) {
                last_continuation = continuation;
                return true;
            };
    }

    {
        quantized_value = 0x001AU;
        auto record = make_record();
        auto state = make_state(0x001AU);
        callbacks.dispatch_field_map_event = [](
            const FieldMapCollisionEventInput& input)
            -> std::optional<FieldMapEventPolicyDecision> {
            return make_script_decision(
                input, FieldMapEventAction::mark_projected_terrain,
                std::nullopt, std::nullopt, false, true);
        };
        callbacks.mark_projected_terrain_cell = [] { return false; };
        const auto not_marked = advance_entity_movement_and_mode_transition(
            record, 0x200U, 0x400U, state, callbacks);
        assert(not_marked.kind ==
                   EntityMovementModeTransitionResultKind::
                       terrain_cell_not_marked &&
               not_marked.record_released &&
               state.projected_terrain_event_published);

        record = make_record();
        state = make_state(0x001AU);
        callbacks.mark_projected_terrain_cell = [] { return true; };
        const auto marked = advance_entity_movement_and_mode_transition(
            record, 0x200U, 0x400U, state, callbacks);
        assert(marked.kind ==
                   EntityMovementModeTransitionResultKind::
                       projected_terrain_marked &&
               marked.continuation ==
                   MovementModeTransitionContinuation::projected_terrain_event &&
               last_continuation ==
                   MovementModeTransitionContinuation::projected_terrain_event);
    }

    {
        quantized_value = 0;
        auto record = make_record();
        auto state = make_state(0xFFC2U);
        state.scene_id = 0x14U;
        state.world_activity_source_word = 0xCAFEU;
        callbacks.dispatch_field_map_event = [](
            const FieldMapCollisionEventInput& input)
            -> std::optional<FieldMapEventPolicyDecision> {
            return make_script_decision(
                input, FieldMapEventAction::begin_maglock_interaction,
                FieldCollisionEventCode::event_2);
        };
        const auto result = advance_entity_movement_and_mode_transition(
            record, 0x200U, 0x400U, state, callbacks);
        assert(result.kind ==
                   EntityMovementModeTransitionResultKind::
                       field_dialogue_started &&
               state.activity_selection == 2U &&
               state.activity_source_word == 0xCAFEU &&
               result.event_code == FieldCollisionEventCode::event_2 &&
               record.projected_field_code == 0xFFC2U &&
               result.continuation ==
                   MovementModeTransitionContinuation::maglock_door_interaction);
    }

    {
        quantized_value = 0;
        auto record = make_record();
        auto state = make_state(0x0018U);
        callbacks.dispatch_field_map_event = [](
            const FieldMapCollisionEventInput& input)
            -> std::optional<FieldMapEventPolicyDecision> {
            return make_script_decision(
                input, FieldMapEventAction::begin_security_terminal);
        };
        const auto result = advance_entity_movement_and_mode_transition(
            record, 0x200U, 0x400U, state, callbacks);
        assert(result.kind ==
                   EntityMovementModeTransitionResultKind::
                       building_reaction_started &&
               result.continuation ==
                   MovementModeTransitionContinuation::building_security_terminal);
    }

    {
        quantized_value = 0;
        auto record = make_record();
        auto state = make_state(0x0019U);
        state.field_transition_context = 0x13U;
        callbacks.dispatch_field_map_event = [](
            const FieldMapCollisionEventInput& input)
            -> std::optional<FieldMapEventPolicyDecision> {
            return make_script_decision(
                input, FieldMapEventAction::begin_event_25_activity);
        };
        const auto result = advance_entity_movement_and_mode_transition(
            record, 0x200U, 0x400U, state, callbacks);
        assert(result.kind ==
                   EntityMovementModeTransitionResultKind::
                       world_activity_started &&
               result.continuation ==
                   MovementModeTransitionContinuation::event_25_world_activity);
    }

    {
        quantized_value = 0;
        auto record = make_record();
        auto state = make_state(0x0019U);
        callbacks.project_entity_world_interaction = [] { return false; };
        callbacks.dispatch_field_map_event = [](
            const FieldMapCollisionEventInput& input)
            -> std::optional<FieldMapEventPolicyDecision> {
            return make_script_decision(
                input, FieldMapEventAction::project_world_interaction,
                FieldCollisionEventCode::event_8, 168U, true, false, true,
                -88, true, 168U);
        };
        const auto result = advance_entity_movement_and_mode_transition(
            record, 0x200U, 0x400U, state, callbacks);
        assert(result.kind == EntityMovementModeTransitionResultKind::
                                  projected_field_code_updated &&
               result.event_code == FieldCollisionEventCode::event_8 &&
               state.mode_event_code == 8U &&
               result.mode_event_code_published &&
               record.projected_field_code == 0xA8U &&
               !result.record_released && (record.route_flags & 0x20U) != 0U);
    }

    {
        quantized_value = 0;
        auto record = make_record();
        auto state = make_state(0x0019U);
        state.field_global_flags = 0x10U;
        state.mode_event_code = 0x0EU;
        callbacks.prepare_mode_entry_service = [] {};
        callbacks.dispatch_field_map_event = [](
            const FieldMapCollisionEventInput& input)
            -> std::optional<FieldMapEventPolicyDecision> {
            return make_script_decision(
                input, FieldMapEventAction::begin_field_mode_exit);
        };
        const auto result = advance_entity_movement_and_mode_transition(
            record, 0x200U, 0x400U, state, callbacks);
        assert(result.kind ==
                   EntityMovementModeTransitionResultKind::mode_exit_started &&
               result.continuation ==
                   MovementModeTransitionContinuation::field_mode_exit_finalizer &&
               !result.mode_event_code_published && state.mode_event_code == 0x0EU &&
               result.record_released);
    }

    {
        quantized_value = 0;
        auto record = make_record();
        auto state = make_state();
        callbacks.dispatch_field_map_event = [](
            const FieldMapCollisionEventInput& input)
            -> std::optional<FieldMapEventPolicyDecision> {
            return make_script_decision(
                input, FieldMapEventAction::project_world_interaction);
        };
        callbacks.project_entity_world_interaction = [] { return true; };
        const auto result = advance_entity_movement_and_mode_transition(
            record, 0x200U, 0x400U, state, callbacks);
        assert(result.kind == EntityMovementModeTransitionResultKind::
                                  projected_interaction_pending &&
               state.world_interaction_lock &&
               state.interaction_transition_pending &&
               state.interaction_transition_timer == 0x10U &&
               !result.record_released);
    }

    {
        quantized_value = 0;
        auto record = make_record();
        auto state = make_state();
        state.world_mode = 2U;
        state.interaction_gate = 1U;
        callbacks.dispatch_field_map_event = [](
            const FieldMapCollisionEventInput& input)
            -> std::optional<FieldMapEventPolicyDecision> {
            return make_script_decision(
                input, FieldMapEventAction::skip_world_interaction,
                std::nullopt, std::nullopt, true);
        };
        const auto result = advance_entity_movement_and_mode_transition(
            record, 0x200U, 0x400U, state, callbacks);
        assert(result.kind ==
                   EntityMovementModeTransitionResultKind::projection_skipped &&
               result.record_released);
    }

    {
        auto record = make_record();
        auto state = make_state();
        const auto result = advance_entity_movement_and_mode_transition(
            record, 0x200U, 0x400U, state, {});
        assert(result.kind ==
               EntityMovementModeTransitionResultKind::callback_unresolved);
    }

    assert(projection_preparations >= 10);
    return 0;
}
