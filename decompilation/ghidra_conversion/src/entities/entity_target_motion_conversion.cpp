#include "shadowrun_ghidra/entities/entity_target_motion_conversion.hpp"

#include "shadowrun_ghidra/entities/entity_state_primitives_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_metric_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

EntityTargetMotionResult no_target_result() {
    return EntityTargetMotionResult{
        EntityTargetMotionAction::no_linked_target};
}

} // namespace

EntityTargetMotionResult resolve_entity_target_motion(
    EntityTargetMotionState& entity,
    const std::optional<LinkedEntityTarget>& target,
    std::array<std::uint8_t, 18>& effect_slots) {
    if (!target.has_value()) {
        return no_target_result();
    }

    const auto measurement = measure_entity_target_heading_and_distance(
        entity.world_x,
        entity.world_y,
        target->world_x,
        target->world_y);

    if (entity.entity_type == 0x000CU) {
        if (measurement.distance >= 0x300U) {
            if (measurement.distance >= 0x600U) {
                return EntityTargetMotionResult{
                    EntityTargetMotionAction::target_out_of_range,
                    false,
                    measurement};
            }

            entity.heading = measurement.heading;
            entity.target_heading = measurement.heading;
            return EntityTargetMotionResult{
                EntityTargetMotionAction::medium_heading_applied,
                true,
                measurement};
        }

        if (entity.effect_gate >= 0) {
            entity.target_heading = measurement.heading;
            return EntityTargetMotionResult{
                EntityTargetMotionAction::close_heading_applied,
                true,
                measurement};
        }

        const auto allocation = allocate_first_free_effect_slot_record(effect_slots);
        if (allocation.table_full) {
            return EntityTargetMotionResult{
                EntityTargetMotionAction::close_effect_table_full,
                false,
                measurement};
        }

        entity.effect_slot = allocation.slot;
        entity.flags = static_cast<std::uint8_t>(entity.flags | 0x40U);
        entity.heading = measurement.heading;
        entity.target_heading = measurement.heading;
        entity.effect_record_parameter = allocation.record_parameter;
        const auto result = EntityTargetMotionResult{
            EntityTargetMotionAction::close_effect_started,
            true,
            measurement,
            std::nullopt,
            allocation.slot};
        return result;
    }

    if (entity.entity_type != 0x000DU && entity.entity_type != 0x000EU) {
        if ((target->flags & 0x01U) == 0U ||
            (target->flags & 0x40U) != 0U ||
            measurement.distance >= 0x200U) {
            return EntityTargetMotionResult{
                EntityTargetMotionAction::target_out_of_range,
                false,
                std::nullopt};
        }

        entity.target_heading = measurement.heading;
        return EntityTargetMotionResult{
            EntityTargetMotionAction::target_heading_applied,
            true,
            measurement};
    }

    const auto projection = project_heading_position(
        entity.world_x, entity.world_y, entity.heading);
    const auto linked_heading = update_entity_heading_from_linked_target(
        true,
        target->flags,
        measurement.heading,
        entity.world_x,
        entity.world_y,
        target->world_x,
        target->world_y);
    if (!linked_heading.has_value()) {
        return EntityTargetMotionResult{
            EntityTargetMotionAction::target_out_of_range,
            false,
            measurement,
            projection};
    }

    if (entity.entity_type == 0x000EU || entity.movement_mode != 2) {
        entity.heading = *linked_heading;
        entity.target_heading = *linked_heading;
        return EntityTargetMotionResult{
            EntityTargetMotionAction::linked_heading_applied,
            true,
            measurement,
            projection};
    }

    // 0x15C84 has written +0x34, while 0x14CD2 restores +0x2E to the
    // pre-projection heading. Type 0xD in movement mode 2 keeps the old
    // current heading but retains the requested heading.
    entity.target_heading = *linked_heading;
    entity.movement_timer = 0x1000U;
    return EntityTargetMotionResult{
        EntityTargetMotionAction::linked_heading_rejected,
        false,
        measurement,
        projection};
}

} // namespace shadowrun::ghidra_conversion
