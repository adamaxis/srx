#include "shadowrun_ghidra/entities/entity_animation_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityAnimationState steady{3U, 1, 2, 8, false};
    advance_entity_animation_accumulator(steady);
    if (!expect(steady.frame == 3U && steady.accumulator == 3 &&
                !steady.redraw_requested)) {
        return 1;
    }

    EntityAnimationState stepped{7U, 6, 3, 8, false};
    advance_entity_animation_accumulator(stepped);
    if (!expect(stepped.frame == 8U && stepped.accumulator == 1 &&
                stepped.redraw_requested)) {
        return 2;
    }

    // The source performs one subtraction per call rather than looping until
    // the accumulator is below the period.
    EntityAnimationState one_step{0xFFFFU, 12, 20, 8, false};
    advance_entity_animation_accumulator(one_step);
    if (!expect(one_step.frame == 0U && one_step.accumulator == 24 &&
                one_step.redraw_requested)) {
        return 3;
    }

    const auto equipment = select_entity_motion_action_row(0, 3U);
    if (!expect(equipment.table == EntityMotionActionTable::EquipmentItem &&
                equipment.one_based_action_code == 3U &&
                equipment.row_byte_offset == 0x60U)) {
        return 4;
    }
    const auto magic = select_entity_motion_action_row(-1, 1U);
    if (!expect(magic.table == EntityMotionActionTable::MagicSpell &&
                magic.one_based_action_code == 1U &&
                magic.row_byte_offset == 0U)) {
        return 5;
    }
    const auto zero_id = select_entity_motion_action_row(0, 0U);
    if (!expect(zero_id.row_byte_offset == 0xFFD0U)) {
        return 6;
    }

    if (!expect(select_field_entity_animation_selector(0, false) == 0x0000U &&
                select_field_entity_animation_selector(4, false) == 0x0010U &&
                select_field_entity_animation_selector(0, true) == 0x0060U &&
                select_field_entity_animation_selector(2, true) == 0x0068U &&
                select_field_entity_animation_selector(7, true) == 0x007CU &&
                entity_kind_state_table(kFieldEntityKind) == 0x00006378U &&
                entity_kind_state_table(0x000DU) == 0x00006BB8U)) {
        return 7;
    }
    if (!expect(bound_field_walk_animation_frame(0) == 0U &&
                bound_field_walk_animation_frame(1) == 1U &&
                bound_field_walk_animation_frame(2) == 1U)) {
        return 8;
    }

    EntityAnimationState walk{0U, 0, kFieldWalkAnimationIncrement,
                              kFieldEntityAnimationPeriod, false};
    for (int tick = 0; tick < 5; ++tick) {
        advance_entity_animation_accumulator(walk);
    }
    if (!expect(walk.frame == 0U && !walk.redraw_requested)) {
        return 9;
    }
    advance_entity_animation_accumulator(walk);
    if (!expect(walk.frame == 1U && walk.redraw_requested &&
                bound_field_walk_animation_frame(walk.frame) == 1U)) {
        return 10;
    }

    const EntitySpriteDescriptorList walk_list{
        static_cast<std::int16_t>(0x8002), 0x0300U, {0x1000U, 0x1004U}};
    EntitySpriteDescriptorState descriptor{
        0x0003U, 0x0060U, 0x01U, 0U, 0U, 0x0010U, 0U};
    if (!expect(advance_entity_sprite_descriptor(
                    descriptor, walk_list, 0x8002U) &&
                descriptor.frame == 0U &&
                descriptor.increment == 0x0300U &&
                (descriptor.selector & 0x8000U) == 0U &&
                (descriptor.flags & 0x01U) == 0U &&
                descriptor.descriptor == 0x1000U)) {
        return 11;
    }

    descriptor = EntitySpriteDescriptorState{
        0x0003U, 0x0060U, 0x01U, 2U, 0U, 0x0010U, 0U};
    if (!expect(advance_entity_sprite_descriptor(
                    descriptor, walk_list, 0x8002U) &&
                descriptor.frame == 1U &&
                descriptor.increment == 0x0300U &&
                (descriptor.selector & 0x8000U) != 0U &&
                descriptor.descriptor == 0x1004U)) {
        return 12;
    }

    const EntitySpriteDescriptorList positive_list{
        4, 0x00FFU, {0x2000U, 0x2004U, 0x2008U, 0x200CU}};
    descriptor = EntitySpriteDescriptorState{
        0x0001U, 0U, 0U, 4U, 0U, 0x0015U, 0U};
    if (!expect(!advance_entity_sprite_descriptor(
                     descriptor, positive_list, 0U) &&
                descriptor.frame == 0U &&
                descriptor.increment == 0x024CU &&
                descriptor.descriptor == 0x2000U &&
                descriptor.increment != 0x00FFU)) {
        return 13;
    }

    descriptor = EntitySpriteDescriptorState{
        0x0001U, 0U, 0U, 1U, 0U, 0x0016U, 0U};
    if (!expect(!advance_entity_sprite_descriptor(
                     descriptor, positive_list, 0U) &&
                descriptor.frame == 1U &&
                descriptor.increment == 0x0160U &&
                descriptor.descriptor == 0x2004U)) {
        return 14;
    }

    descriptor = EntitySpriteDescriptorState{
        0x000DU, 0U, 0U, 2U, 0U, 0x0016U, 0U};
    if (!expect(!advance_entity_sprite_descriptor(
                     descriptor, positive_list, 0U) &&
                descriptor.increment == 0x0090U)) {
        return 15;
    }
    descriptor = EntitySpriteDescriptorState{
        0x000DU, 0U, 0U, 1U, 0U, 0x0016U, 0U};
    if (!expect(!advance_entity_sprite_descriptor(
                     descriptor, positive_list, 0U) &&
                descriptor.increment == 0x0180U)) {
        return 16;
    }

    descriptor = EntitySpriteDescriptorState{
        0x0003U, 0x0060U, 0U, 0U, 0U, 0U, 0U};
    if (!expect(!advance_entity_sprite_descriptor(
                     descriptor, walk_list, 0x8002U) &&
                (descriptor.flags & 0x01U) == 0U &&
                descriptor.descriptor == 0x1000U)) {
        return 17;
    }
    return 0;
}
