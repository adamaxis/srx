#include "shadowrun_ghidra/entities/entity_effect_spawn_conversion.hpp"

#include "shadowrun_ghidra/entities/movement_metric_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

std::size_t direction_index(const std::uint16_t heading) {
    auto rotated = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(heading << 3U) |
        static_cast<std::uint16_t>(heading >> 13U));
    if ((rotated & 0x8000U) != 0U) {
        ++rotated;
    }
    return static_cast<std::size_t>(rotated & 0x0007U);
}

std::uint16_t add_word(
    const std::uint16_t value,
    const std::int16_t delta) {
    return static_cast<std::uint16_t>(
        static_cast<std::int32_t>(value) + delta);
}

struct DirectionOffsets {
    std::int16_t position_x = 0;
    std::int16_t position_y = 0;
    std::int16_t probe_x = 0;
    std::int16_t probe_y = 0;
};

DirectionOffsets direction_offsets(const std::uint16_t heading) {
    // ROM 0x15A4C (position) / 0x15A6C (FFDD60/62 probe): signed words,
    // indexed by the rotated heading used by 0x14FF8. Gameplay copy lives in
    // gameplay/scripts/effect_spawn.lua.
    static constexpr std::int16_t kPositionOffsets[8][2] = {
        {0x0000, -0x00B0}, {0x007C, -0x007C},
        {0x00B0, 0x0000}, {0x007C, 0x007C},
        {0x0000, 0x00B0}, {-0x007C, 0x007C},
        {-0x00B0, 0x0000}, {-0x007C, -0x007C}};
    static constexpr std::int16_t kProbeOffsets[8][2] = {
        {0x0040, -0x0100}, {0x0120, -0x00F0},
        {0x0180, -0x0040}, {0x00D0, 0x0050},
        {0x0000, 0x0020}, {-0x0180, 0x0040},
        {-0x0180, -0x0040}, {-0x0100, -0x00C0}};
    const auto index = direction_index(heading);
    return DirectionOffsets{
        kPositionOffsets[index][0], kPositionOffsets[index][1],
        kProbeOffsets[index][0], kProbeOffsets[index][1]};
}

} // namespace

EntityEffectSpawnResult spawn_entity_effect_record(
    const EntityEffectSpawnParent& parent,
    const std::optional<EntityEffectSpawnTarget> linked_target,
    std::array<std::uint8_t, 18>& effect_slots,
    const EntityEffectSpawnCallbacks& callbacks) {
    // 0x15646: the source masks the parent's animation counter before the
    // unsigned <= 0x3C window test. The host keeps the carry exit explicit.
    if ((parent.animation_counter & 0x07FFU) > 0x003CU) {
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

    EntityEffectSpawnRecord record{};
    initialize_entity_behavior_record(record.behavior_defaults);
    record.entity_slot = *entity_slot;
    record.world_x = parent.world_x;
    record.world_y = parent.world_y;
    record.heading = parent.heading;
    record.movement_scale = 0x2000U;
    record.animation_resource = 0U;
    record.field_0x56 = parent.field_0x56;
    record.field_0x57 = parent.field_0x57;
    record.field_0x9D = parent.field_0x9D;
    record.continuation_address = 0x00015390U;
    record.lifetime = 0x0013U;
    record.movement_offset = static_cast<std::int16_t>(0xFB00U);
    record.vertical_velocity = 0xF400U;
    record.behavior_type = 0x02U;
    record.effect_distance_steps = 0x00B4U;
    record.active = true;

    const auto allocation = allocate_first_free_effect_slot_record(effect_slots);
    if (allocation.table_full) {
        // 0x156E6 clears the new record's +0x10 active byte before returning
        // through the carry path; the allocated entity slot is not reused by
        // this routine.
        record.active = false;
        return EntityEffectSpawnResult{
            EntityEffectSpawnAction::effect_table_full, true, record};
    }
    record.effect_slot = allocation.slot;
    record.effect_record_parameter = allocation.record_parameter;
    record.effect_auxiliary_offset = allocation.record_auxiliary_offset;

    const auto offsets = direction_offsets(record.heading);
    record.directional_probe_x = add_word(record.world_x, offsets.probe_x);
    record.directional_probe_y = add_word(record.world_y, offsets.probe_y);
    record.world_x = add_word(record.world_x, offsets.position_x);
    record.world_y = add_word(record.world_y, offsets.position_y);

    if (!linked_target.has_value()) {
        record.source_marker = static_cast<std::uint8_t>(
            parent.field_0x87 >> 1U);
        const auto offsets_from_heading = derive_scaled_entity_heading_offsets(
            record.heading, 0x0008, 5U);
        record.heading_offset_x = offsets_from_heading.horizontal;
        record.heading_offset_y = offsets_from_heading.vertical;
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

    record.source_marker = resolved_target->source_marker;
    auto measurement = measure_relative_heading_and_distance(
        parent.world_x,
        parent.world_y,
        resolved_target->target.world_x,
        resolved_target->target.world_y);
    if (measurement.distance > 0x0BE0U) {
        measurement.distance = 0x0BE0U;
    }
    record.heading = measurement.heading;
    record.effect_distance_steps = static_cast<std::uint16_t>(
        (measurement.distance / 0x0080U) * 2U);
    record.movement_offset = static_cast<std::int16_t>(
        -static_cast<std::int32_t>(measurement.distance / 0x0080U) * 0x20);
    const auto offsets_from_heading = derive_scaled_entity_heading_offsets(
        record.heading, 0x0008, 4U);
    record.heading_offset_x = offsets_from_heading.horizontal;
    record.heading_offset_y = offsets_from_heading.vertical;
    return EntityEffectSpawnResult{
        EntityEffectSpawnAction::spawned_toward_target, false, record};
}

} // namespace shadowrun::ghidra_conversion
