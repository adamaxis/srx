#include "shadowrun_ghidra/entities/entity_behavior_transition_completion_conversion.hpp"

#include <cassert>

using namespace shadowrun::ghidra_conversion;

int main() {
    EntityBehaviorFrameRecord record{};
    record.identity = 0x21U;
    record.route_flags = 0x80U;
    record.behavior_flags = 0x20U;
    record.spawn_state_flags = 0x40U;

    EntityBehaviorTransitionWorldState world{};
    world.scene_spawn_countdown = 1U;
    world.deferred_world_activity = [&] { world.world_activity_countdown = 7U; };
    world.field_entity_flags = 0x40U;
    world.primary_runner_flags = 0x40U;
    world.recruited_runner_flags = 0U;
    world.linked_records[1] = {true, 0x21U, 0U};
    world.linked_records[2] = {true, 0x22U, 0U};

    int begin_calls = 0;
    int spawn_calls = 0;
    int reset_calls = 0;
    const auto result = complete_entity_behavior_transition(
        record, world,
        {[&] { ++begin_calls; },
         [&](EntityBehaviorFrameRecord& spawned_record) {
             ++spawn_calls;
             assert(&spawned_record == &record);
             return true;
         },
         [&] { ++reset_calls; }});

    assert(result.kind == EntityBehaviorTransitionResultKind::completed);
    assert(result.entity_transition_activity_started && begin_calls == 1);
    assert(result.scene_entity_spawned && spawn_calls == 1);
    assert(result.service_records_reset && reset_calls == 1);
    assert(result.deferred_activity_started && world.deferred_world_activity ==
                                                   nullptr);
    assert(world.world_activity_countdown == 7U);
    assert((record.behavior_flags & 0x40U) != 0U);
    assert(result.linked_records_marked == 1U);
    assert((world.linked_records[1].behavior_flags & 0x20U) != 0U);
    assert((world.linked_records[2].behavior_flags & 0x20U) == 0U);

    EntityBehaviorFrameRecord gate_record{};
    gate_record.behavior_flags = 0x08U;
    EntityBehaviorTransitionWorldState gate_world{};
    gate_world.field_entity_flags = 0x40U;
    gate_world.primary_runner_flags = 0U;
    gate_world.recruited_runner_flags = 0x40U;
    const auto armed = complete_entity_behavior_transition(
        gate_record, gate_world, {});
    assert(armed.kind == EntityBehaviorTransitionResultKind::completed);
    assert(armed.world_activity_countdown_armed &&
           gate_world.world_activity_countdown == 0x3CU);

    EntityBehaviorFrameRecord missing_spawn{};
    EntityBehaviorTransitionWorldState missing_world{};
    const auto unresolved = complete_entity_behavior_transition(
        missing_spawn, missing_world, {});
    assert(unresolved.kind ==
           EntityBehaviorTransitionResultKind::callback_unresolved);
    assert(missing_spawn.behavior_flags == 0U);
    return 0;
}
