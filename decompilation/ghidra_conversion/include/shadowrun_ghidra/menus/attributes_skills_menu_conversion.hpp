#pragma once

#include "shadowrun_ghidra/session/runner_record_operations_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>

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

enum class AttributeSkillUpgradeStatus {
    available,
    invalid_selection,
    attribute_not_upgradeable,
    unknown_species,
    class_restricted,
    at_limit,
};

struct AttributeSkillUpgradePreview {
    AttributeSkillUpgradeStatus status =
        AttributeSkillUpgradeStatus::invalid_selection;
    AttributeSkillSection section = AttributeSkillSection::attribute;
    std::uint8_t value_index = 0;
    std::uint16_t record_offset = 0;
    std::uint8_t current_value = 0;
    std::uint8_t next_value = 0;
    std::uint16_t karma_cost = 0;
};

enum class AttributeSkillUpgradeResult {
    upgraded,
    confirmation_required,
    insufficient_karma,
    invalid_selection,
    attribute_not_upgradeable,
    unknown_species,
    class_restricted,
    at_limit,
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

enum class AttributeSkillRenderOutcome {
    rendered_upgrade_preview,
    invalid_upgrade_selection,
    selection_data_unavailable,
};

struct AttributeSkillRenderResult {
    AttributeSkillRenderOutcome outcome =
        AttributeSkillRenderOutcome::selection_data_unavailable;
    AttributeSkillCell selected_cell{};
    AttributeSkillUpgradePreview upgrade{};
    std::uint8_t karma_available = 0;
};

struct AttributeSkillRenderCallbacks {
    // ROM 0xD82A starts by clearing FFF0B3 and submitting the fixed
    // Attributes/Skills background grid (D4=CA86, D1=4, D7=21).
    std::function<void()> clear_upgrade_error_marker;
    std::function<void()> write_attributes_skills_grid;

    // ROM 0xDA4A is reached only after the selection-specific source gate;
    // the callback names the static Attributes/Skills heading it renders.
    std::function<void()> render_attributes_skills_heading;
    std::function<void(const AttributeSkillCell&)> render_selected_label;

    // D9DE renders the invalid-upgrade message. On the valid path D9C2 emits
    // the Karma cost field before D9D4 emits the next value field.
    std::function<void()> render_invalid_upgrade_message;
    std::function<void(std::uint8_t)> render_upgrade_cost;
    std::function<void(std::uint8_t)> render_next_value;

    // The common DA00 tail renders the selected-label pointer, the three-row
    // overlay grid, and the active record's +0x8D Karma value in that order.
    std::function<void()> render_selection_overlay_grid;
    std::function<void(std::uint8_t)> render_karma_total;
};

// ROM 0x0D82A: rebuild the Attributes/Skills presentation, evaluate the
// selected upgrade branch, and execute the common selected-label/overlay/Karma
// tail. This is a typed presentation choreography; it does not reproduce the
// source's VDP destinations or RAM marker bytes.
AttributeSkillRenderResult render_attributes_and_skills(
    const RunnerRecordFields& record,
    AttributeSkillCursor cursor,
    bool selection_data_available,
    const AttributeSkillRenderCallbacks& callbacks = {});

const AttributeSkillCell& selected_attribute_skill_cell(
    const AttributeSkillGrid& grid, AttributeSkillCursor cursor);

// ROM 0x0D01C keeps the cursor in a two-column, ten-row surface. Horizontal
// movement selects a column and vertical movement clamps to rows 0..9; it
// does not wrap a column or row at the edge.
AttributeSkillCursor move_attribute_skill_cursor(
    AttributeSkillCursor cursor, bool left, bool right, bool up, bool down);

// ROM 0x0D82A computes the next value/cost shown beside the selected cell.
// Attribute limits are the five six-byte species rows at 0xC22A6. Attribute
// cost is the next attribute value; skill costs use the source's doubled or
// left-column 1.5x branches. The available Karma byte is +0x8D.
AttributeSkillUpgradePreview preview_attribute_skill_upgrade(
    const RunnerRecordFields& record, AttributeSkillCursor cursor);

// ROM 0x0D3EA applies the confirmed upgrade: subtract the previewed Karma
// cost from +0x8D and write the next value to the selected +0x77..+0x8C
// record field. `confirmed=false` represents the still-open Upgrade? prompt.
AttributeSkillUpgradeResult apply_attribute_skill_upgrade(
    RunnerRecordFields& record, AttributeSkillCursor cursor, bool confirmed);

} // namespace shadowrun::ghidra_conversion
