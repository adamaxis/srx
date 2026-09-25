#include "shadowrun_ghidra/attributes_skills_menu_conversion.hpp"

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    RunnerRecordFields record{};
    record.archetype_index = 2U;
    record.attributes = {10U, 11U, 12U, 13U, 14U, 15U, 16U, 17U};
    record.skills = {20U, 21U, 22U, 23U, 24U, 25U,
                    26U, 27U, 28U, 29U, 30U, 31U};

    const auto grid = read_attributes_and_skills_grid(record);
    const auto& left_top = grid[0][0];
    if (!expect(left_top.section == AttributeSkillSection::attribute &&
                left_top.label_index == 0U && left_top.value_index == 0U &&
                left_top.record_offset == 0x77U && left_top.value == 10U)) {
        return 1;
    }

    const auto& right_attribute = grid[1][3];
    if (!expect(right_attribute.label_index == 7U &&
                right_attribute.value_index == 7U &&
                right_attribute.record_offset == 0x7EU &&
                right_attribute.value == 17U)) {
        return 2;
    }

    const auto& left_skill = grid[0][4];
    const auto& right_skill = grid[1][9];
    if (!expect(left_skill.section == AttributeSkillSection::skill &&
                left_skill.label_index == 8U && left_skill.value_index == 0U &&
                left_skill.record_offset == 0x80U && left_skill.value == 20U &&
                right_skill.label_index == 19U &&
                right_skill.value_index == 11U &&
                right_skill.record_offset == 0x8CU &&
                right_skill.value == 31U)) {
        return 3;
    }

    record.archetype_index = 2U;
    const auto restricted = read_attributes_and_skills_grid(record);
    if (!expect(restricted[0][4].state ==
                    AttributeSkillCellState::class_restricted &&
                restricted[1][4].state == AttributeSkillCellState::value)) {
        return 4;
    }

    const auto absent = read_attributes_and_skills_grid(record, false);
    if (!expect(absent[0][0].state ==
                    AttributeSkillCellState::no_live_record &&
                absent[0][4].state ==
                    AttributeSkillCellState::no_live_record)) {
        return 5;
    }

    AttributeSkillCursor cursor{0U, 0U};
    cursor = move_attribute_skill_cursor(cursor, false, true, false, false);
    cursor = move_attribute_skill_cursor(cursor, false, false, false, true);
    cursor = move_attribute_skill_cursor(cursor, false, false, false, true);
    if (!expect(cursor.column == 1U && cursor.row == 2U)) {
        return 6;
    }
    cursor = move_attribute_skill_cursor(cursor, true, false, true, false);
    cursor = move_attribute_skill_cursor(cursor, true, false, true, false);
    cursor = move_attribute_skill_cursor(cursor, true, false, true, false);
    if (!expect(cursor.column == 0U && cursor.row == 0U)) {
        return 7;
    }

    cursor = move_attribute_skill_cursor({1U, 9U}, false, false, false, true);
    if (!expect(cursor.column == 1U && cursor.row == 9U &&
                &selected_attribute_skill_cell(grid, cursor) == &grid[1][9])) {
        return 8;
    }

    return 0;
}
