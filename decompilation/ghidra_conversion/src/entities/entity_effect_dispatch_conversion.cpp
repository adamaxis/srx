#include "shadowrun_ghidra/entities/entity_effect_dispatch_conversion.hpp"

#include "shadowrun_ghidra/entities/entity_state_primitives_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_metric_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

constexpr std::uint16_t kNearbyDistanceLimit = 0x0500U;

std::uint16_t source_word_delta(
    const std::uint16_t left,
    const std::uint16_t right) {
    return static_cast<std::uint16_t>(left - right);
}

std::uint16_t audio_event_for_behavior(const std::uint8_t behavior_type) {
    // ROM 0x4946: the source checks this table before calling
    // play_sfx_if_enabled. Zero means that the behavior has no sound
    // event at this dispatch point.
    static constexpr std::array<std::uint16_t, 16> kAudioEvents{
        0x0000U, 0x0032U, 0x0034U, 0x0034U,
        0x0031U, 0x0037U, 0x0038U, 0x0000U,
        0x0033U, 0x0035U, 0x0035U, 0x0039U,
        0x003AU, 0x0036U, 0x0000U, 0x0000U};
    return behavior_type < kAudioEvents.size()
               ? kAudioEvents[behavior_type]
               : 0U;
}

bool is_target_scan_behavior(const std::uint8_t behavior_type) {
    return behavior_type == 0x03U || behavior_type == 0x05U ||
           behavior_type == 0x06U || behavior_type == 0x0AU ||
           behavior_type == 0x0DU;
}

bool selects_nearby_target(const std::uint8_t behavior_type) {
    return behavior_type == 0x06U || behavior_type == 0x0DU;
}

} // namespace

EntityEffectTargetScanResult scan_entity_effect_targets(
    const EntityEffectDispatchSource& source,
    const std::size_t linked_target_index,
    std::array<EntityEffectDispatchTarget, 10>& entities) {
    EntityEffectTargetScanResult result{};
    if (!is_target_scan_behavior(source.behavior_type) ||
        linked_target_index >= entities.size()) {
        return result;
    }

    result.recognized_behavior = true;
    const auto& linked_target = entities[linked_target_index];
    result.linked_field_0x0A = linked_target.field_0x0A;
    result.linked_field_0x0C = linked_target.field_0x0C;
    bool selected = false;

    for (std::size_t index = 0; index < entities.size(); ++index) {
        auto& candidate = entities[index];
        if (!candidate.active || candidate.excluded) {
            continue;
        }

        const auto distance = compute_diagonal_step_cost(
            static_cast<std::int16_t>(source_word_delta(
                candidate.world_x, linked_target.world_x)),
            static_cast<std::int16_t>(source_word_delta(
                candidate.world_y, linked_target.world_y)));
        if (distance >= kNearbyDistanceLimit) {
            continue;
        }

        candidate.scan_type = source.behavior_type;
        if (source.behavior_type == 0x0DU) {
            candidate.scan_flags = static_cast<std::uint8_t>(
                candidate.scan_flags | 0x02U);
            candidate.parent_source = source.source_entity_id;
        } else if (source.behavior_type == 0x06U) {
            candidate.scan_flags = static_cast<std::uint8_t>(
                candidate.scan_flags | 0x01U);
            candidate.parent_source = source.source_entity_id;
        }

        if (selects_nearby_target(source.behavior_type) && !selected) {
            selected = true;
            candidate.effect_type = source.behavior_type;
            result.selected_nearby_target = index;
        }
    }
    return result;
}

EntityEffectDispatchResult dispatch_entity_effect(
    const EntityEffectDispatchSource& source,
    const std::optional<std::size_t> linked_target_index,
    std::array<EntityEffectDispatchTarget, 10>& entities,
    std::array<std::uint8_t, 18>& effect_slots,
    const EntityEffectDispatchCallbacks& callbacks) {
    if (source.behavior_type == 0x07U || source.behavior_type == 0x0EU) {
        if (callbacks.spawn_direct_effect) {
            callbacks.spawn_direct_effect(source);
        }
        return EntityEffectDispatchResult{
            EntityEffectDispatchAction::direct_spawn_boundary};
    }

    if (!linked_target_index.has_value() ||
        *linked_target_index >= entities.size()) {
        return EntityEffectDispatchResult{
            EntityEffectDispatchAction::no_linked_target};
    }

    if (callbacks.schedule_world_timeout) {
        callbacks.schedule_world_timeout();
    }
    const auto allocation = allocate_first_free_effect_slot_record(effect_slots);
    EntityEffectDispatchResult result{};
    result.allocation = allocation;
    result.allocation_performed = !allocation.table_full;
    if (allocation.table_full) {
        result.action = EntityEffectDispatchAction::effect_table_full;
        return result;
    }

    entities[*linked_target_index].parent_source = source.source_entity_id;
    const auto scan = scan_entity_effect_targets(
        source, *linked_target_index, entities);
    result.recognized_behavior = scan.recognized_behavior;
    result.linked_field_0x0A = scan.linked_field_0x0A;
    result.linked_field_0x0C = scan.linked_field_0x0C;

    const auto target_index = scan.selected_nearby_target.has_value()
                                  ? scan.selected_nearby_target
                                  : (selects_nearby_target(source.behavior_type)
                                         ? std::nullopt
                                         : linked_target_index);
    if (!target_index.has_value()) {
        result.action = EntityEffectDispatchAction::nearby_target_not_found;
        return result;
    }

    auto& target = entities[*target_index];
    target.effect_type = source.behavior_type;
    target.effect_slot = allocation.slot;
    target.effect_record_parameter = allocation.record_parameter;
    target.effect_phase = 5U;
    target.effect_subphase = 0U;
    result.target_index = target_index;

    const auto audio_event = audio_event_for_behavior(source.behavior_type);
    if (audio_event != 0U && callbacks.service_audio) {
        callbacks.service_audio(
            source.behavior_type, audio_event, *target_index);
    }
    if (callbacks.dispatch_effect_behavior) {
        callbacks.dispatch_effect_behavior(source.behavior_type, *target_index);
    }
    result.action = scan.selected_nearby_target.has_value()
                        ? EntityEffectDispatchAction::applied_to_nearby_target
                        : EntityEffectDispatchAction::applied_to_linked_target;
    return result;
}

EntitySecondaryEffectResult apply_entity_secondary_effect(
    const std::size_t target_index,
    std::array<EntityEffectDispatchTarget, 10>& entities,
    std::array<std::uint8_t, 18>& effect_slots,
    const EntityEffectDispatchCallbacks& callbacks) {
    if (target_index >= entities.size()) {
        return EntitySecondaryEffectResult{
            EntitySecondaryEffectAction::invalid_target};
    }

    const auto allocation = allocate_first_free_effect_slot_record(effect_slots);
    EntitySecondaryEffectResult result{};
    result.allocation = allocation;
    result.allocation_performed = !allocation.table_full;
    if (allocation.table_full) {
        result.action = EntitySecondaryEffectAction::effect_table_full;
        return result;
    }

    auto& target = entities[target_index];
    target.effect_type = 0x02U;
    target.effect_slot = allocation.slot;
    target.effect_record_parameter = allocation.record_parameter;
    target.effect_phase = 0U;
    target.effect_subphase = 0U;
    if (callbacks.service_audio) {
        callbacks.service_audio(0x02U, 0x0034U, target_index);
    }
    if (callbacks.dispatch_effect_behavior) {
        callbacks.dispatch_effect_behavior(0x02U, target_index);
    }
    result.action = EntitySecondaryEffectAction::applied;
    return result;
}

} // namespace shadowrun::ghidra_conversion
