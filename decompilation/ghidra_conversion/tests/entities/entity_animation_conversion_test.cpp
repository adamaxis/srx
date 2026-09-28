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
                select_field_entity_animation_selector(7, true) == 0x007CU)) {
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

    return 0;
}
