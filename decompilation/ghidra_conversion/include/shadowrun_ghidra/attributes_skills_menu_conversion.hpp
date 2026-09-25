#pragma once

#include "shadowrun_ghidra/runner_record_operations_conversion.hpp"

#include <array>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kAttributesSkillsColumnCount = 2U;
constexpr std::size_t kAttributesSkillsRowCount = 10U;

enum class AttributeSkillSection {
    attribute,
    skill,
};

enum class AttributeSkillCellState {
    value,
    no_live_record,
    class_restricted,
};

struct AttributeSkillCursor {
    std::uint8_t column = 0;
    std::uint8_t row = 0;
};

struct AttributeSkillCell {
    AttributeSkillSection section = AttributeSkillSection::attribute;
    // Index into the source C1A2A label table: eight attributes followed by
    // twelve skills.
    std::uint8_t label_index = 0;
    // Index in RunnerRecordFields::attributes or ::skills.
    std::uint8_t value_index = 0;
    // Source byte offset in the live 0x100-byte runner record.
    std::uint16_t record_offset = 0;
    std::uint8_t value = 0;
    AttributeSkillCellState state = AttributeSkillCellState::value;
};

using AttributeSkillGrid =
    std::array<std::array<AttributeSkillCell, kAttributesSkillsRowCount>,
               kAttributesSkillsColumnCount>;

struct AttributeSkillMenuState {
    AttributeSkillValues values{};
    bool live_record_available = true;
    AttributeSkillCursor cursor{};
};

// ROM 0x0D82A lays out the active record as two columns of ten rows. The
// first four rows read +0x77..+0x7E; the lower six rows read +0x80..+0x85
// in the left column and +0x87..+0x8C in the right column. Label indices
// follow the same source order in the C1A2A pointer table.
AttributeSkillGrid build_attributes_and_skills_grid(
    const AttributeSkillMenuState& state);

AttributeSkillGrid read_attributes_and_skills_grid(
    const RunnerRecordFields& record, bool live_record_available = true);

const AttributeSkillCell& selected_attribute_skill_cell(
    const AttributeSkillGrid& grid, AttributeSkillCursor cursor);

// ROM 0x0D01C keeps the cursor in a two-column, ten-row surface. Horizontal
// movement selects a column and vertical movement clamps to rows 0..9; it
// does not wrap a column or row at the edge.
AttributeSkillCursor move_attribute_skill_cursor(
    AttributeSkillCursor cursor, bool left, bool right, bool up, bool down);

} // namespace shadowrun::ghidra_conversion
