#include "shadowrun_ghidra/entity_target_motion_conversion.hpp"

#include <array>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::array<std::uint8_t, 18> effects{};
    EntityTargetMotionState close_entity{
        0x0000U, 0x0000U, 0U, 0U, 0U, 0x000CU, 0, -1, 0U, 0U, 0U};
    const auto close_effect = resolve_entity_target_motion(
        close_entity,
        LinkedEntityTarget{0x0064U, 0U, 0x01U},
        effects);
    if (!expect(close_effect.action ==
                    EntityTargetMotionAction::close_effect_started &&
                close_effect.allocated_effect_slot.has_value() &&
                close_effect.allocated_effect_slot.value() == 0U &&
                close_entity.effect_slot == 0U &&
                (close_entity.flags & 0x40U) != 0U &&
                close_entity.heading == 0x8000U &&
                close_entity.target_heading == 0x8000U &&
                close_entity.effect_record_parameter == 0x0049U &&
                close_effect.source_carry)) {
        return 1;
    }

    EntityTargetMotionState close_heading_entity{
        0x0000U, 0x0000U, 0x1234U, 0x5678U, 0U, 0x000CU, 0, 0, 0U, 0U, 0U};
    const auto close_heading = resolve_entity_target_motion(
        close_heading_entity,
        LinkedEntityTarget{0x0064U, 0U, 0x01U},
        effects);
    if (!expect(close_heading.action ==
                    EntityTargetMotionAction::close_heading_applied &&
                close_heading.source_carry &&
                close_heading_entity.heading == 0x1234U &&
                close_heading_entity.target_heading == 0x8000U)) {
        return 7;
    }

    EntityTargetMotionState medium_entity{
        0x0000U, 0x0000U, 0U, 0U, 0U, 0x000CU, 0, 0, 0U, 0U, 0U};
    const auto medium = resolve_entity_target_motion(
        medium_entity,
        LinkedEntityTarget{0x0400U, 0U, 0x01U},
        effects);
    if (!expect(medium.action ==
                    EntityTargetMotionAction::medium_heading_applied &&
                medium.source_carry && medium_entity.heading == 0x8000U &&
                medium_entity.target_heading == 0x8000U)) {
        return 2;
    }

    EntityTargetMotionState far_entity{
        0x0000U, 0x0000U, 0x1234U, 0x5678U, 0U, 0x000CU, 0, 0, 0U, 0U, 0U};
    const auto far = resolve_entity_target_motion(
        far_entity,
        LinkedEntityTarget{0x0600U, 0U, 0x01U},
        effects);
    if (!expect(far.action == EntityTargetMotionAction::target_out_of_range &&
                !far.source_carry && far_entity.heading == 0x1234U &&
                far_entity.target_heading == 0x5678U)) {
        return 3;
    }

    EntityTargetMotionState linked_entity{
        0x0000U, 0x0000U, 0x0000U, 0U, 0U, 0x000EU, 2, 0, 0U, 0U, 0U};
    const auto linked = resolve_entity_target_motion(
        linked_entity,
        LinkedEntityTarget{0x0064U, 0U, 0x01U},
        effects);
    if (!expect(linked.action ==
                    EntityTargetMotionAction::linked_heading_applied &&
                linked.source_carry && linked.projected_position.has_value() &&
                linked_entity.heading == 0x8000U)) {
        return 4;
    }

    EntityTargetMotionState restricted_entity{
        0x0000U, 0x0000U, 0x0000U, 0U, 0U, 0x000DU, 2, 0, 0U, 0U, 0U};
    const auto restricted = resolve_entity_target_motion(
        restricted_entity,
        LinkedEntityTarget{0x0064U, 0U, 0x01U},
        effects);
    if (!expect(restricted.action ==
                EntityTargetMotionAction::linked_heading_rejected &&
                !restricted.source_carry &&
                restricted_entity.movement_timer == 0x1000U &&
                restricted_entity.heading == 0x0000U &&
                restricted_entity.target_heading == 0x8000U)) {
        return 5;
    }

    EntityTargetMotionState generic_entity{
        0x0000U, 0x0000U, 0U, 0U, 0U, 0x0001U, 0, 0, 0U, 0U, 0U};
    const auto generic = resolve_entity_target_motion(
        generic_entity,
        LinkedEntityTarget{0x0064U, 0U, 0x01U},
        effects);
    if (!expect(generic.action ==
                    EntityTargetMotionAction::target_heading_applied &&
                generic.source_carry && generic_entity.target_heading ==
                    0x8000U)) {
        return 6;
    }

    return 0;
}
