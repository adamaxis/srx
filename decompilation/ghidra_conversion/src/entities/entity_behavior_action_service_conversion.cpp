#include "shadowrun_ghidra/entities/entity_behavior_action_service_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

bool source_type_uses_alternate_effect_heading(const std::uint8_t source_type) {
    return source_type >= 0x0DU && source_type < 0x10U;
}

bool source_type_uses_spawn_cycle(const std::uint8_t source_type) {
    // The 0x0D..0x0F effect family is selected by the preceding branch. The
    // ordinary spawn-cycle path therefore covers source types 1..0x0C.
    return source_type > 0U && source_type < 0x0DU;
}

void prepare_collision_recheck(EntityBehaviorFrameRecord& record) {
    record.record_flags = static_cast<std::uint8_t>(
        record.record_flags | 0x01U);
    record.continuation_kind = EntityBehaviorContinuationKind::collision_recheck;
}

EntityBehaviorActionResult finish_with_controller_action(
    EntityBehaviorFrameRecord& record,
    EntityBehaviorActionWorldState& world) {
    EntityBehaviorActionResult result{};
    if (world.controller_action != -1) {
        record.target_heading = static_cast<std::uint16_t>(
            world.controller_action);
        record.continuation_kind = EntityBehaviorContinuationKind::movement_route;
        result.kind = EntityBehaviorActionResultKind::controller_heading_prepared;
        return result;
    }

    if (record.continuation_kind ==
        EntityBehaviorContinuationKind::collision_recheck) {
        result.kind = EntityBehaviorActionResultKind::collision_recheck_prepared;
        return result;
    }
    prepare_collision_recheck(record);
    result.kind = EntityBehaviorActionResultKind::collision_recheck_prepared;
    return result;
}

} // namespace

bool entity_behavior_action_callbacks_complete(
    const EntityBehaviorFrameRecord& record,
    const EntityBehaviorActionWorldState& world,
    const EntityBehaviorActionCallbacks& callbacks) {
    if ((world.controller_events & 0x1000U) != 0U &&
        !callbacks.select_world_entity_service_target) {
        return false;
    }
    if (world.service_suppressed || world.movement_transition_active ||
        world.activity_lock) {
        return true;
    }

    if (record.transition_gate >= 0) {
        return true;
    }

    if (world.action_override == 0 &&
        (world.controller_events & 0x0040U) == 0U &&
        world.controller_action == -1 &&
        !callbacks.update_entity_animation_delay) {
        return false;
    }
    if (record.target_marker < 0) {
        return callbacks.update_entity_target_heading &&
               (world.alternate_delayed_animation
                    ? static_cast<bool>(callbacks.prepare_delayed_animation_alternate)
                    : static_cast<bool>(callbacks.prepare_delayed_animation_primary));
    }
    if (source_type_uses_alternate_effect_heading(record.source_type)) {
        return callbacks.spawn_entity_effect_record &&
               callbacks.set_entity_heading_with_alternate_animation;
    }
    if (record.source_type == 0U) {
        return static_cast<bool>(callbacks.configure_entity_target_transition);
    }
    if (source_type_uses_spawn_cycle(record.source_type)) {
        return callbacks.process_entity_spawn_cycle &&
               callbacks.set_entity_heading_with_field_animation &&
               callbacks.dispatch_player_action_if_ready &&
               callbacks.service_audio_if_enabled &&
               callbacks.update_entity_animation_delay;
    }
    return true;
}

EntityBehaviorActionResult advance_entity_behavior_action(
    EntityBehaviorFrameRecord& record,
    EntityBehaviorActionWorldState& world,
    const EntityBehaviorActionCallbacks& callbacks) {
    EntityBehaviorActionResult result{};
    if (!entity_behavior_action_callbacks_complete(record, world, callbacks)) {
        result.kind = EntityBehaviorActionResultKind::callback_unresolved;
        return result;
    }

    if ((world.controller_events & 0x1000U) != 0U &&
        callbacks.select_world_entity_service_target) {
        callbacks.select_world_entity_service_target(record);
        result.service_target_selected = true;
    }

    if (world.service_suppressed) {
        prepare_collision_recheck(record);
        result.kind = EntityBehaviorActionResultKind::service_suppressed;
        return result;
    }

    if (world.movement_transition_active) {
        const auto heading = static_cast<std::uint16_t>(
            record.action_heading_source - static_cast<std::uint16_t>(0x8000U));
        record.current_heading = heading;
        record.target_heading = heading;
        record.continuation_kind = EntityBehaviorContinuationKind::movement_route;
        result.kind =
            EntityBehaviorActionResultKind::movement_transition_redirected;
        return result;
    }

    if (world.activity_lock) {
        if (world.activity_countdown != 0U) {
            world.activity_countdown = static_cast<std::uint8_t>(
                world.activity_countdown - static_cast<std::uint8_t>(1U));
            prepare_collision_recheck(record);
            result.kind = EntityBehaviorActionResultKind::activity_countdown_waiting;
            return result;
        }
        record.target_heading = 0U;
        record.continuation_kind = EntityBehaviorContinuationKind::movement_route;
        result.kind = EntityBehaviorActionResultKind::activity_countdown_expired;
        return result;
    }

    if (record.transition_gate >= 0) {
        if ((world.controller_events & 0x4000U) != 0U) {
            world.action_override = -1;
            result.action_override_set = true;
        }
        return finish_with_controller_action(record, world);
    }

    if (world.action_override == 0 &&
        (world.controller_events & 0x0040U) == 0U) {
        if (world.controller_action == -1) {
            callbacks.update_entity_animation_delay(record, 1U);
            record.auxiliary_marker = static_cast<std::uint8_t>(
                record.auxiliary_marker + static_cast<std::uint8_t>(1U));
            result.kind = EntityBehaviorActionResultKind::animation_delay_pending;
            return finish_with_controller_action(record, world);
        }
        record.auxiliary_marker = 0U;
    } else if (world.action_override != 0) {
        world.action_override = 0;
    }
    record.auxiliary_marker = 0U;

    if (record.target_marker < 0) {
        callbacks.update_entity_target_heading(record);
        if (world.alternate_delayed_animation) {
            callbacks.prepare_delayed_animation_alternate(record);
        } else {
            callbacks.prepare_delayed_animation_primary(record);
        }
        result.kind = EntityBehaviorActionResultKind::target_animation_prepared;
        return result;
    }

    if (source_type_uses_alternate_effect_heading(record.source_type)) {
        const auto spawned = callbacks.spawn_entity_effect_record(record);
        if (spawned.source_carry) {
            result.kind = EntityBehaviorActionResultKind::effect_spawn_blocked;
            return result;
        }
        callbacks.set_entity_heading_with_alternate_animation(
            record, spawned.spawned_heading);
        result.kind = EntityBehaviorActionResultKind::effect_heading_prepared;
        return result;
    }

    if (record.source_type == 0U) {
        callbacks.configure_entity_target_transition(record);
        result.kind = EntityBehaviorActionResultKind::target_transition_prepared;
        return result;
    }

    if (record.source_type > 0x0EU) {
        result.kind = EntityBehaviorActionResultKind::no_action;
        return result;
    }

    const auto spawn_cycle = callbacks.process_entity_spawn_cycle(record);
    if (!spawn_cycle.source_carry) {
        callbacks.set_entity_heading_with_field_animation(
            record, spawn_cycle.spawned_heading);
        result.kind =
            EntityBehaviorActionResultKind::spawn_cycle_heading_prepared;
        return result;
    }

    result.action_dispatched = callbacks.dispatch_player_action_if_ready(record);
    if (!result.action_dispatched) {
        prepare_collision_recheck(record);
        result.kind = EntityBehaviorActionResultKind::collision_recheck_prepared;
        return result;
    }

    result.audio_event = 0x004BU;
    callbacks.service_audio_if_enabled(result.audio_event);
    callbacks.update_entity_animation_delay(record, 0U);
    result.kind = finish_with_controller_action(record, world).kind;
    return result;
}

} // namespace shadowrun::ghidra_conversion
