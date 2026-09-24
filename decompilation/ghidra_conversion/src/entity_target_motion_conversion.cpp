#include "shadowrun_ghidra/entity_target_motion_conversion.hpp"

#include "shadowrun_ghidra/entity_state_primitives_conversion.hpp"
#include "shadowrun_ghidra/movement_metric_conversion.hpp"

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
    std::array<std::uint8_t, 18>& effect_slots,
    const std::uint16_t effect_auxiliary_value) {
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
                false,
                measurement};
        }

        const auto slot = allocate_first_free_effect_slot(effect_slots);
        if (slot == effect_slots.size()) {
            return EntityTargetMotionResult{
                EntityTargetMotionAction::close_effect_table_full,
                false,
                measurement};
        }

        entity.effect_slot = slot;
        entity.flags = static_cast<std::uint8_t>(entity.flags | 0x40U);
        entity.target_heading = measurement.heading;
        entity.effect_auxiliary_value = effect_auxiliary_value;
        const auto result = EntityTargetMotionResult{
            EntityTargetMotionAction::close_effect_started,
            false,
            measurement,
            std::nullopt,
            slot};
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

    entity.heading = *linked_heading;
    entity.target_heading = *linked_heading;
    if (entity.entity_type == 0x000EU || entity.movement_mode != 2) {
        return EntityTargetMotionResult{
            EntityTargetMotionAction::linked_heading_applied,
            true,
            measurement,
            projection};
    }

    entity.movement_timer = 0x1000U;
    return EntityTargetMotionResult{
        EntityTargetMotionAction::linked_heading_rejected,
        false,
        measurement,
        projection};
}

} // namespace shadowrun::ghidra_conversion
