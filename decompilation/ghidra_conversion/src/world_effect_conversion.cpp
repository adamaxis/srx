#include "shadowrun_ghidra/world_effect_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::uint16_t find_first_available_world_effect_entity_slot(
    const std::array<std::uint8_t, 7>& active_flags) {
    for (std::size_t index = 0; index < active_flags.size(); ++index) {
        if ((active_flags[index] & 0x01U) == 0U) {
            return static_cast<std::uint16_t>(index + 3U);
        }
    }
    return 10U;
}

bool schedule_world_activity_timeout(WorldActivityTimeoutState& state) {
    if (state.scene_id == 0x0036U && !state.scene_ready) {
        return false;
    }

    if (state.timer_marker == 0) {
        state.timer_marker = -1;
    }
    state.timeout = state.long_timeout_source ? 0x0258U : 0x0078U;
    return true;
}

void clear_entity_effect_pending_flags(
    std::array<std::uint8_t, 6>& effect_flags) {
    for (auto& flags : effect_flags) {
        flags = static_cast<std::uint8_t>(flags & 0xFEU);
    }
}

void promote_entity_effect_flags(std::array<std::uint8_t, 6>& effect_flags) {
    for (auto& flags : effect_flags) {
        if ((flags & 0x02U) != 0U) {
            flags = static_cast<std::uint8_t>(flags | 0x04U);
        }
    }
}

WorldEffectEntitySpawnResult spawn_world_effect_entities(
    const std::vector<WorldEffectSpawnDescriptor>& descriptors,
    const std::function<std::optional<std::uint16_t>()>& allocate_entity_slot,
    const std::function<std::optional<std::uint16_t>()>& allocate_effect_slot,
    const WorldEffectEntityInitializer& initialize_entity,
    const WorldEffectEntityProjection& project_entity) {
    WorldEffectEntitySpawnResult result{};
    std::int16_t vertical_origin = 0;
    std::optional<std::uint16_t> previous_entity_slot;

    for (const auto& descriptor : descriptors) {
        if (descriptor.sentinel) {
            break;
        }
        if (!allocate_entity_slot) {
            result.action = WorldEffectEntitySpawnAction::entity_table_full;
            break;
        }
        const auto entity_slot = allocate_entity_slot();
        if (!entity_slot.has_value()) {
            result.action = WorldEffectEntitySpawnAction::entity_table_full;
            break;
        }
        if (!allocate_effect_slot) {
            result.action = WorldEffectEntitySpawnAction::effect_table_full;
            break;
        }
        const auto effect_slot = allocate_effect_slot();
        if (!effect_slot.has_value()) {
            result.action = WorldEffectEntitySpawnAction::effect_table_full;
            break;
        }

        const auto parent_entity_slot =
            descriptor.reset_vertical_origin ? std::nullopt
                                             : previous_entity_slot;
        if (descriptor.reset_vertical_origin) {
            vertical_origin = 0;
        } else {
            vertical_origin = static_cast<std::int16_t>(
                vertical_origin + 0x00C0);
        }

        auto entity = initialize_entity(
            descriptor,
            *entity_slot,
            *effect_slot,
            static_cast<std::uint16_t>(vertical_origin),
            parent_entity_slot);
        if (descriptor.reset_vertical_origin) {
            entity.overlay_flag = true;
            previous_entity_slot = *entity_slot;
        }
        entity.effect_active = true;
        if (project_entity) {
            entity.visible = project_entity(entity);
        }
        if (entity.visible) {
            entity.transition_code = 4U;
            entity.world_x = descriptor.destination_x;
            entity.world_y = descriptor.destination_y;
        }
        result.entities.push_back(entity);
    }
    result.final_vertical_origin = vertical_origin;
    return result;
}

WorldEffectSingleSpawnResult spawn_world_effect_from_coordinate_sequence(
    WorldEffectCoordinateSequenceState& state,
    const WorldEffectEntitySlotAllocator& allocate_entity_slot,
    const WorldEffectEntitySlotAllocator& allocate_effect_slot,
    const WorldEffectRecordBoundary& initialize_record,
    const WorldEffectBoundary& schedule_timeout) {
    WorldEffectSingleSpawnResult result{};
    if (!allocate_entity_slot) {
        result.action = WorldEffectSingleSpawnAction::entity_table_full;
        return result;
    }
    result.entity_slot = allocate_entity_slot();
    if (!result.entity_slot.has_value()) {
        result.action = WorldEffectSingleSpawnAction::entity_table_full;
        return result;
    }
    if (!allocate_effect_slot) {
        result.action = WorldEffectSingleSpawnAction::effect_table_full;
        return result;
    }
    result.effect_slot = allocate_effect_slot();
    if (!result.effect_slot.has_value()) {
        result.action = WorldEffectSingleSpawnAction::effect_table_full;
        return result;
    }

    const auto read_coordinate = [&state]() {
        if (state.cursor >= state.coordinates.size()) {
            return state.fallback;
        }
        return state.coordinates[state.cursor++];
    };
    auto coordinate = read_coordinate();
    auto y_coordinate = coordinate;
    if (coordinate.first == 0U) {
        coordinate = state.fallback;
        y_coordinate = state.fallback;
    } else {
        y_coordinate = read_coordinate();
    }
    result.world_x = coordinate.first;
    result.world_y = y_coordinate.second;
    if (initialize_record) {
        initialize_record(*result.entity_slot,
                          *result.effect_slot,
                          result.world_x,
                          result.world_y,
                          true);
    }
    state.active_count = static_cast<std::uint8_t>(state.active_count + 1U);
    result.active_count = state.active_count;
    if (schedule_timeout) {
        schedule_timeout();
    }
    return result;
}

WorldEffectSingleSpawnResult spawn_world_effect_at_player_position(
    const std::uint16_t player_x,
    const std::uint16_t player_y,
    const WorldEffectEntitySlotAllocator& allocate_entity_slot,
    const WorldEffectEntitySlotAllocator& allocate_effect_slot,
    const WorldEffectRecordBoundary& initialize_record) {
    WorldEffectSingleSpawnResult result{};
    if (!allocate_entity_slot) {
        result.action = WorldEffectSingleSpawnAction::entity_table_full;
        return result;
    }
    result.entity_slot = allocate_entity_slot();
    if (!result.entity_slot.has_value()) {
        result.action = WorldEffectSingleSpawnAction::entity_table_full;
        return result;
    }
    if (!allocate_effect_slot) {
        result.action = WorldEffectSingleSpawnAction::effect_table_full;
        return result;
    }
    result.effect_slot = allocate_effect_slot();
    if (!result.effect_slot.has_value()) {
        result.action = WorldEffectSingleSpawnAction::effect_table_full;
        return result;
    }

    result.world_x = static_cast<std::uint16_t>(player_x + 0x0080U);
    result.world_y = player_y;
    if (initialize_record) {
        initialize_record(*result.entity_slot,
                          *result.effect_slot,
                          result.world_x,
                          result.world_y,
                          true);
    }
    return result;
}

WorldEffectTransitionAction begin_world_effect_transition(
    WorldEffectTransitionState& state,
    WorldActivityTimeoutState& timeout_state,
    const WorldSoundBoundary& play_sound,
    const WorldEffectBoundary& clear_pending_flags,
    const WorldEffectBoundary& process_effect_list) {
    if (state.scene_id == 0x0014U) {
        static_cast<void>(schedule_world_activity_timeout(timeout_state));
        return WorldEffectTransitionAction::timeout_scheduled;
    }

    if (state.world_mode != 2U) {
        return WorldEffectTransitionAction::no_action;
    }

    state.transition_timer = 0x0960U;
    if (state.transition_latched) {
        return WorldEffectTransitionAction::already_latched;
    }

    state.transition_latched = true;
    if (play_sound) {
        play_sound(0x0030U);
    }
    if (clear_pending_flags) {
        clear_pending_flags();
    }
    if (process_effect_list) {
        process_effect_list();
    }
    return WorldEffectTransitionAction::started;
}

WorldEffectTriggerAction advance_world_effect_trigger(
    WorldEffectTriggerState& state,
    std::array<std::uint8_t, 6>& effect_flags,
    const std::vector<WorldEffectRegion>& regions,
    const WorldRandomBoundary& next_random,
    WorldActivityTimeoutState& timeout_state,
    const WorldSoundBoundary& play_sound,
    const WorldEffectBoundary& finish_transition_sound,
    const WorldEffectBoundary& clear_pending_flags,
    const WorldEffectBoundary& process_effect_list) {
    if (state.transition.transition_latched) {
        --state.transition.transition_timer;
        if (state.transition.transition_timer < 0) {
            state.transition.transition_latched = false;
            promote_entity_effect_flags(effect_flags);
            if (finish_transition_sound) {
                finish_transition_sound();
            }
            return WorldEffectTriggerAction::transition_finished;
        }
    }

    if (state.trigger_suppressed) {
        return WorldEffectTriggerAction::trigger_suppressed;
    }

    if ((state.frame_counter & 0x003FU) != 0x000DU) {
        return WorldEffectTriggerAction::cadence_wait;
    }

    if (!next_random || next_random() > 0x7FFFU) {
        return WorldEffectTriggerAction::random_rejected;
    }

    const auto player_x = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(state.player_fixed_x >> 4U));
    const auto player_y = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(state.player_fixed_y >> 4U));
    bool matching_region = false;
    for (const auto& region : regions) {
        if (region.sentinel) {
            break;
        }
        if (player_x > region.minimum_x &&
            player_x <= region.maximum_x &&
            player_y > region.minimum_y &&
            player_y <= region.maximum_y) {
            matching_region = true;
            break;
        }
    }
    if (!matching_region) {
        return WorldEffectTriggerAction::no_matching_region;
    }

    const auto transition_action = begin_world_effect_transition(
        state.transition,
        timeout_state,
        play_sound,
        clear_pending_flags,
        process_effect_list);
    switch (transition_action) {
    case WorldEffectTransitionAction::timeout_scheduled:
        return WorldEffectTriggerAction::timeout_scheduled;
    case WorldEffectTransitionAction::already_latched:
        return WorldEffectTriggerAction::transition_already_latched;
    case WorldEffectTransitionAction::started:
        return WorldEffectTriggerAction::transition_started;
    case WorldEffectTransitionAction::no_action:
        return WorldEffectTriggerAction::no_action;
    }
    return WorldEffectTriggerAction::no_action;
}

} // namespace shadowrun::ghidra_conversion
