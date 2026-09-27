#include "shadowrun_ghidra/entities/entity_behavior_transition_completion_conversion.hpp"

#include <utility>

namespace shadowrun::ghidra_conversion {
namespace {

bool spawn_path_reachable(const EntityBehaviorFrameRecord& record) {
    return (record.behavior_flags & 0x08U) == 0U;
}

bool countdown_can_reset_service_records(
    const EntityBehaviorFrameRecord& record,
    const EntityBehaviorTransitionWorldState& world) {
    return spawn_path_reachable(record) &&
           (record.spawn_state_flags & 0x40U) != 0U &&
           world.scene_spawn_countdown == 1U;
}

} // namespace

bool entity_behavior_transition_callbacks_complete(
    const EntityBehaviorFrameRecord& record,
    const EntityBehaviorTransitionWorldState& world,
    const EntityBehaviorTransitionCallbacks& callbacks) {
    if ((record.route_flags & 0x80U) != 0U &&
        !callbacks.begin_entity_transition_activity) {
        return false;
    }
    if (spawn_path_reachable(record)) {
        if (!callbacks.spawn_scene_entity_from_weighted_table) {
            return false;
        }
    }
    // The spawn routine determines whether the completion flag is set, so a
    // reachable spawn path must have its reset boundary available before the
    // callback runs. This keeps a later source branch from becoming a silent
    // no-op when the callback is only missing on that outcome.
    if (spawn_path_reachable(record) &&
        !callbacks.reset_entity_service_records) {
        return false;
    }
    return true;
}

EntityBehaviorTransitionResult complete_entity_behavior_transition(
    EntityBehaviorFrameRecord& record,
    EntityBehaviorTransitionWorldState& world,
    const EntityBehaviorTransitionCallbacks& callbacks) {
    EntityBehaviorTransitionResult result{};
    if (!entity_behavior_transition_callbacks_complete(record, world,
                                                       callbacks)) {
        result.kind = EntityBehaviorTransitionResultKind::callback_unresolved;
        return result;
    }

    if ((record.route_flags & 0x80U) != 0U) {
        callbacks.begin_entity_transition_activity();
        result.entity_transition_activity_started = true;
    }

    if (spawn_path_reachable(record)) {
        result.scene_entity_spawned =
            callbacks.spawn_scene_entity_from_weighted_table(record);
        if ((record.spawn_state_flags & 0x40U) != 0U) {
            world.scene_spawn_countdown = static_cast<std::uint8_t>(
                world.scene_spawn_countdown - static_cast<std::uint8_t>(1U));
            if (world.scene_spawn_countdown == 0U) {
                callbacks.reset_entity_service_records();
                result.service_records_reset = true;
                if (world.deferred_world_activity) {
                    auto deferred_activity =
                        std::move(world.deferred_world_activity);
                    deferred_activity();
                    result.deferred_activity_started = true;
                }
            }
        }
    }

    // ROM 0xAB6E marks completion before the optional six-record link scan.
    record.behavior_flags = static_cast<std::uint8_t>(
        record.behavior_flags | 0x40U);

    if ((record.behavior_flags & 0x20U) != 0U) {
        for (auto& linked_record : world.linked_records) {
            if (linked_record.active &&
                linked_record.owner_identity == record.identity) {
                linked_record.behavior_flags = static_cast<std::uint8_t>(
                    linked_record.behavior_flags | 0x20U);
                ++result.linked_records_marked;
            }
        }
    }

    if ((record.behavior_flags & 0x08U) != 0U &&
        (world.field_entity_flags & 0x40U) != 0U &&
        ((world.primary_runner_flags & 0x01U) == 0U ||
         (world.primary_runner_flags & 0x40U) != 0U) &&
        ((world.recruited_runner_flags & 0x01U) == 0U ||
         (world.recruited_runner_flags & 0x40U) != 0U)) {
        world.world_activity_countdown = 0x003CU;
        result.world_activity_countdown_armed = true;
    }

    return result;
}

} // namespace shadowrun::ghidra_conversion
