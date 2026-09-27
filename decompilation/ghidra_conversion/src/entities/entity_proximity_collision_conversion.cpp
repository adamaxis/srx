#include "shadowrun_ghidra/entities/entity_proximity_collision_conversion.hpp"

#include "shadowrun_ghidra/entities/movement_metric_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

std::uint16_t add_word(const std::uint16_t value, const std::uint16_t delta) {
    return static_cast<std::uint16_t>(value + delta);
}

bool points_to(const std::optional<std::size_t>& link,
              const std::size_t target_index) {
    return link.has_value() && link.value() == target_index;
}

bool within_axis_window(const std::uint16_t value,
                        const std::uint16_t center,
                        const std::uint16_t radius) {
    const auto lower = static_cast<std::uint16_t>(center - radius);
    const auto upper = add_word(center, radius);
    return value >= lower && value < upper;
}

void publish_pair_interaction(EntityProximityRecord& current,
                              EntityProximityRecord& candidate) {
    const auto delta_x = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(candidate.x - current.x));
    const auto delta_y = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(candidate.y - current.y));
    const auto measurement = measure_heading_and_distance(delta_x, delta_y);
    candidate.interaction_heading = static_cast<std::uint16_t>(
        measurement.heading + 0x4000U);
    current.interaction_heading = static_cast<std::uint16_t>(
        measurement.heading - 0x4000U);
    candidate.interaction_value = current.behavior_value;
    current.interaction_value = candidate.behavior_value;
}

void begin_normal_activity(
    EntityPairCollisionWorldState& world,
    const EntityPairCollisionCallbacks& callbacks,
    EntityPairCollisionResult& result) {
    world.published_transfer_value = world.source_transfer_value;
    if (world.world_effect_active) {
        world.world_effect_active = false;
        world.handler = EntityPairActivityHandler::world_effect_collision;
    } else if (world.pending_world_effect) {
        world.pending_world_effect = false;
        world.handler =
            EntityPairActivityHandler::pending_world_effect_collision;
    } else {
        world.handler = EntityPairActivityHandler::normal_collision;
    }
    world.world_activity_started = true;
    result.normal_activity_requested = true;
    result.handler = world.handler;
    if (callbacks.begin_world_activity) {
        callbacks.begin_world_activity();
    }
}

} // namespace

EntityProximityInteractionState resolve_entity_proximity_interactions(
    std::vector<EntityProximityRecord>& records,
    const std::size_t current_index,
    const EntityProximityInteractionCallbacks& callbacks) {
    EntityProximityInteractionState state{};
    if (current_index >= records.size()) {
        return state;
    }

    auto& current = records[current_index];
    if ((current.entity_flags & 0x40U) != 0U || current.player_record) {
        state.skipped_current_record = true;
        return state;
    }

    for (std::size_t candidate_index = current_index + 1U;
         candidate_index < records.size(); ++candidate_index) {
        auto& candidate = records[candidate_index];
        ++state.pairs_examined;
        if ((candidate.entity_flags & 0x01U) == 0U ||
            (candidate.entity_flags & 0x40U) != 0U) {
            continue;
        }

        const bool linked =
            points_to(candidate.primary_link, current_index) ||
            points_to(current.primary_link, candidate_index) ||
            points_to(candidate.secondary_link, current_index) ||
            points_to(current.secondary_link, candidate_index);
        const auto radius = linked ? static_cast<std::uint16_t>(0x0100U)
                                   : static_cast<std::uint16_t>(0x0040U);
        if (!within_axis_window(current.x, candidate.x, radius) ||
            !within_axis_window(current.y, candidate.y, radius)) {
            continue;
        }

        current.collision_flags =
            static_cast<std::uint8_t>(current.collision_flags | 0x02U);
        candidate.collision_flags =
            static_cast<std::uint8_t>(candidate.collision_flags | 0x02U);
        publish_pair_interaction(current, candidate);
        if (callbacks.resolve_pair_collision) {
            callbacks.resolve_pair_collision(current, candidate);
        }
        ++state.pairs_resolved;

        if (callbacks.advance_random &&
            (callbacks.advance_random() & 0x7000U) == 0U) {
            candidate.interaction_heading = static_cast<std::uint16_t>(
                candidate.interaction_heading + 0x4000U);
            current.interaction_heading = static_cast<std::uint16_t>(
                current.interaction_heading - 0x4000U);
            candidate.interaction_value = static_cast<std::uint16_t>(
                candidate.interaction_value + candidate.interaction_value);
            current.interaction_value = static_cast<std::uint16_t>(
                current.interaction_value + current.interaction_value);
        }
    }
    return state;
}

EntityPairCollisionResult resolve_entity_pair_collision_event(
    EntityPairCollisionEntity& current,
    EntityPairCollisionEntity& candidate,
    EntityPairCollisionWorldState& world,
    const EntityPairCollisionCallbacks& callbacks) {
    EntityPairCollisionResult result{};

    const auto begin_special_activity = [&](const EntityPairActor actor) {
        result.special_activity_requested = true;
        world.selected_activity_actor = actor;
        if (world.activity_in_progress) {
            return;
        }

        world.handler = EntityPairActivityHandler::special_collision;
        if (world.scene_activity_id == 10U) {
            world.activity_variant = world.field_mode == 6U ? 2U : 4U;
            world.handler = world.field_mode == 6U
                ? EntityPairActivityHandler::special_mode_six_collision
                : EntityPairActivityHandler::special_collision;
        } else if (world.scene_activity_id >= 10U) {
            const auto table_index = static_cast<std::size_t>(
                world.scene_activity_id - 10U);
            if (table_index < world.scene_activity_table.size()) {
                world.activity_variant = world.scene_activity_table[table_index];
                world.handler =
                    EntityPairActivityHandler::special_scene_table_collision;
            }
        }
        world.world_activity_started = true;
        result.handler = world.handler;
        result.activity_variant = world.activity_variant;
        if (callbacks.begin_world_activity_if_idle) {
            callbacks.begin_world_activity_if_idle(actor);
        }
    };

    if ((candidate.special_flags & 0x10U) != 0U) {
        if ((current.entity_flags & 0x02U) != 0U) {
            begin_special_activity(EntityPairActor::candidate);
        }
    } else if ((current.special_flags & 0x10U) != 0U &&
               (candidate.entity_flags & 0x02U) != 0U) {
        begin_special_activity(EntityPairActor::current);
    }

    if ((candidate.special_flags & 0x08U) != 0U) {
        if ((candidate.activity_flags & 0x80U) != 0U ||
            (current.entity_flags & 0x02U) == 0U) {
            return result;
        }
        candidate.activity_flags = static_cast<std::uint8_t>(
            candidate.activity_flags | 0x80U);
        result.flagged_entity = EntityPairActor::candidate;
    } else if ((current.special_flags & 0x08U) != 0U) {
        if ((current.activity_flags & 0x80U) != 0U ||
            (candidate.entity_flags & 0x02U) == 0U) {
            return result;
        }
        current.activity_flags = static_cast<std::uint8_t>(
            current.activity_flags | 0x80U);
        result.flagged_entity = EntityPairActor::current;
    } else {
        return result;
    }

    begin_normal_activity(world, callbacks, result);
    result.activity_variant = world.activity_variant;
    return result;
}

} // namespace shadowrun::ghidra_conversion
