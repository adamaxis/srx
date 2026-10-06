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
        auto state = make_state();
        state.mode_event_code = 0x12U;
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
        const auto result = advance_entity_movement_and_mode_transition(
            record, 0x200U, 0x400U, state, callbacks);
        assert(result.kind ==
                   EntityMovementModeTransitionResultKind::projection_skipped &&
               result.record_released && last_continuation ==
                   MovementModeTransitionContinuation::event_31_world_activity);
    }

    {
        quantized_value = 0x001AU;
        auto record = make_record();
        auto state = make_state(0x001AU);
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
        const auto result = advance_entity_movement_and_mode_transition(
            record, 0x200U, 0x400U, state, callbacks);
        assert(result.kind ==
                   EntityMovementModeTransitionResultKind::
                       field_dialogue_started &&
               state.activity_selection == 2U &&
               state.activity_source_word == 0xCAFEU &&
               result.event_code == 2U &&
               record.projected_field_code == 0xFFC2U &&
               result.continuation ==
                   MovementModeTransitionContinuation::maglock_door_interaction);
    }

    {
        quantized_value = 0;
        auto record = make_record();
        auto state = make_state(0x0018U);
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
        const auto result = advance_entity_movement_and_mode_transition(
            record, 0x200U, 0x400U, state, callbacks);
        assert(result.kind == EntityMovementModeTransitionResultKind::
                                  projected_field_code_updated &&
               result.event_code == 8U && state.mode_event_code == 8U &&
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
