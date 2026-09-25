#include "shadowrun_ghidra/attributes_skills_menu_conversion.hpp"

#include <algorithm>
#include <cstddef>

namespace shadowrun::ghidra_conversion {
namespace {

struct SourceCell {
    AttributeSkillSection section;
    std::uint8_t label_index;
    std::uint8_t value_index;
    std::uint16_t record_offset;
};

SourceCell source_cell(const std::size_t column, const std::size_t row) {
    if (row < 4U) {
        const auto value_index = static_cast<std::uint8_t>(
            row + column * 4U);
        return {AttributeSkillSection::attribute, value_index, value_index,
                static_cast<std::uint16_t>(0x77U + value_index)};
    }

    const auto skill_index = static_cast<std::uint8_t>(
        (row - 4U) + column * 6U);
    const auto skill_offset = static_cast<std::uint16_t>(
        skill_index < 6U ? 0x80U + skill_index : 0x87U + skill_index - 6U);
    return {AttributeSkillSection::skill,
            static_cast<std::uint8_t>(8U + skill_index), skill_index,
            skill_offset};
}

} // namespace

AttributeSkillGrid build_attributes_and_skills_grid(
    const AttributeSkillMenuState& state) {
    AttributeSkillGrid grid{};
    for (std::size_t column = 0; column < kAttributesSkillsColumnCount;
         ++column) {
        for (std::size_t row = 0; row < kAttributesSkillsRowCount; ++row) {
            const auto source = source_cell(column, row);
            auto& cell = grid[column][row];
            cell.section = source.section;
            cell.label_index = source.label_index;
            cell.value_index = source.value_index;
            cell.record_offset = source.record_offset;
            cell.state = state.live_record_available
                             ? AttributeSkillCellState::value
                             : AttributeSkillCellState::no_live_record;
            if (source.section == AttributeSkillSection::attribute) {
                cell.value = state.values.attributes[source.value_index];
            } else {
                cell.value = state.values.skills[source.value_index];
                // 0xD82A skips the first skill cell when the +0x9B class byte
                // is at least two. This is a source gate,
                // not a missing value and must remain visible to callers.
                if (source.value_index == 0U &&
                    state.values.archetype_index >= 2U &&
                    state.live_record_available) {
                    cell.state = AttributeSkillCellState::class_restricted;
                }
            }
        }
    }
    return grid;
}

AttributeSkillGrid read_attributes_and_skills_grid(
    const RunnerRecordFields& record, const bool live_record_available) {
    AttributeSkillMenuState state{};
    state.values = read_attributes_and_skills(record);
    state.live_record_available = live_record_available;
    return build_attributes_and_skills_grid(state);
}

const AttributeSkillCell& selected_attribute_skill_cell(
    const AttributeSkillGrid& grid, const AttributeSkillCursor cursor) {
    const auto column = std::min<std::size_t>(
        cursor.column, kAttributesSkillsColumnCount - 1U);
    const auto row = std::min<std::size_t>(
        cursor.row, kAttributesSkillsRowCount - 1U);
    return grid[column][row];
}

AttributeSkillCursor move_attribute_skill_cursor(
    AttributeSkillCursor cursor, const bool left, const bool right,
    const bool up, const bool down) {
    if (left && !right) {
        cursor.column = 0U;
    } else if (right && !left) {
        cursor.column = 1U;
    }

    if (up && !down && cursor.row > 0U) {
        --cursor.row;
    } else if (down && !up && cursor.row < 9U) {
        ++cursor.row;
    }
    return cursor;
}

} // namespace shadowrun::ghidra_conversion
