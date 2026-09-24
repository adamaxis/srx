#pragma once

#include "shadowrun_ghidra/entity_route_primitives_conversion.hpp"
#include "shadowrun_ghidra/movement_metric_conversion.hpp"

#include <array>
#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct LinkedEntityTarget {
    std::uint16_t world_x = 0;
    std::uint16_t world_y = 0;
    std::uint8_t flags = 0;
};

struct EntityTargetMotionState {
    std::uint16_t world_x = 0;
    std::uint16_t world_y = 0;
    std::uint16_t heading = 0;
    std::uint16_t target_heading = 0;
    std::uint16_t movement_timer = 0;
    std::uint16_t entity_type = 0;
    std::int16_t movement_mode = 0;
    std::int16_t effect_gate = 0;
    std::uint8_t flags = 0;
    std::uint16_t effect_auxiliary_value = 0;
    std::uint8_t effect_slot = 0;
};

enum class EntityTargetMotionAction {
    no_linked_target,
    target_out_of_range,
    target_heading_applied,
    close_heading_applied,
    close_effect_started,
    close_effect_table_full,
    medium_heading_applied,
    linked_heading_applied,
    linked_heading_rejected,
};

struct EntityTargetMotionResult {
    EntityTargetMotionAction action =
        EntityTargetMotionAction::no_linked_target;
    bool source_carry = false;
    std::optional<RelativeHeadingMeasurement> measurement;
    std::optional<HeadingProjection> projected_position;
    std::optional<std::uint8_t> allocated_effect_slot;
};

// ROM 0x14C7C: apply the entity-type target-motion policy. The state is a
// native record of only the fields this routine writes; source carry exits
// are retained as an explicit result for its caller at 0x13FFE.
EntityTargetMotionResult resolve_entity_target_motion(
    EntityTargetMotionState& entity,
    const std::optional<LinkedEntityTarget>& target,
    std::array<std::uint8_t, 18>& effect_slots,
    std::uint16_t effect_auxiliary_value);

} // namespace shadowrun::ghidra_conversion
