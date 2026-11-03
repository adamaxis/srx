#include "shadowrun_ghidra/entities/entity_effect_spawn_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

void copy_source_decisions(
    EntityEffectSpawnRecord& host_record,
    const EntityEffectSpawnRecord& lua_record) {
    host_record.world_x = lua_record.world_x;
    host_record.world_y = lua_record.world_y;
    host_record.heading = lua_record.heading;
    host_record.movement_scale = lua_record.movement_scale;
    host_record.animation_resource = lua_record.animation_resource;
    host_record.effect_distance_steps = lua_record.effect_distance_steps;
    host_record.movement_offset = lua_record.movement_offset;
    host_record.vertical_velocity = lua_record.vertical_velocity;
    host_record.lifetime = lua_record.lifetime;
    host_record.directional_probe_x = lua_record.directional_probe_x;
    host_record.directional_probe_y = lua_record.directional_probe_y;
    host_record.heading_offset_x = lua_record.heading_offset_x;
    host_record.heading_offset_y = lua_record.heading_offset_y;
    host_record.behavior_type = lua_record.behavior_type;
    host_record.behavior = lua_record.behavior;
    host_record.field_0x56 = lua_record.field_0x56;
    host_record.field_0x57 = lua_record.field_0x57;
    host_record.field_0x9D = lua_record.field_0x9D;
    host_record.source_marker = lua_record.source_marker;
}

} // namespace

EntityEffectSpawnResult spawn_entity_effect_record(
    const EntityEffectSpawnParent& parent,
    const std::optional<EntityEffectSpawnTarget> linked_target,
    std::array<std::uint8_t, 18>& effect_slots,
    const EntityEffectSpawnCallbacks& callbacks) {
    // The source counter policy and source-derived record fields are supplied
    // by Lua; this function only coordinates host allocations and services.
    if (!callbacks.animation_window_open ||
        !callbacks.animation_window_open(parent.animation_counter)) {
        return EntityEffectSpawnResult{
            EntityEffectSpawnAction::animation_window_expired, true};
    }

    if (callbacks.advance_asset_meter) {
        callbacks.advance_asset_meter(-1);
    }
    if (!callbacks.allocate_entity_slot) {
        return EntityEffectSpawnResult{
            EntityEffectSpawnAction::no_entity_slot, true};
    }
    const auto entity_slot = callbacks.allocate_entity_slot();
    if (!entity_slot.has_value()) {
        return EntityEffectSpawnResult{
            EntityEffectSpawnAction::no_entity_slot, true};
    }

    if (callbacks.schedule_world_timeout) {
        callbacks.schedule_world_timeout();
    }

    if (!callbacks.build_record) {
        return EntityEffectSpawnResult{
            EntityEffectSpawnAction::record_policy_unresolved, true};
    }
    const auto base_record = callbacks.build_record(parent, std::nullopt);
    if (!base_record.has_value()) {
        return EntityEffectSpawnResult{
            EntityEffectSpawnAction::record_policy_unresolved, true};
    }

    EntityEffectSpawnRecord record = *base_record;
    initialize_entity_behavior_record(record.behavior_defaults);
    record.entity_slot = *entity_slot;
    record.active = true;

    const auto allocation = allocate_first_free_effect_slot_record(effect_slots);
    if (allocation.table_full) {
        // The ROM allocates the entity before the effect-slot scan, then clears
        // +0x10 and returns carry when all eighteen slots are occupied.
        record.active = false;
        return EntityEffectSpawnResult{
            EntityEffectSpawnAction::effect_table_full, true, record};
    }
    record.effect_slot = allocation.slot;
    record.effect_record_parameter = allocation.record_parameter;
    record.effect_auxiliary_offset = allocation.record_auxiliary_offset;

    if (!linked_target.has_value()) {
        return EntityEffectSpawnResult{
            EntityEffectSpawnAction::spawned_without_target, false, record};
    }

    if (!callbacks.transform_target) {
        return EntityEffectSpawnResult{
            EntityEffectSpawnAction::target_transform_unresolved, true, record};
    }
    const auto resolved_target =
        callbacks.transform_target(*linked_target, parent.heading);
    if (!resolved_target.has_value()) {
        return EntityEffectSpawnResult{
            EntityEffectSpawnAction::target_transform_unresolved, true, record};
    }

    const auto linked_record = callbacks.build_record(parent, resolved_target);
    if (!linked_record.has_value()) {
        return EntityEffectSpawnResult{
            EntityEffectSpawnAction::record_policy_unresolved, true, record};
    }
    copy_source_decisions(record, *linked_record);
    return EntityEffectSpawnResult{
        EntityEffectSpawnAction::spawned_toward_target, false, record};
}

} // namespace shadowrun::ghidra_conversion
