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

struct AttributesSkillsMenuState {
    AttributeSkillCursor cursor{};
    // Source FFF116: negative means the active selection/detail data is not
    // available and an A/B request leaves the menu for field mode.
    bool selection_data_available = false;
    // Source FFF0B3: D82A's invalid-upgrade branch has already presented the
    // rejection, so a later A/B sample waits instead of repeating it.
    bool upgrade_error_marker = false;
    bool open = false;
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

struct AttributesSkillsMenuInitializationCallbacks {
    // ROM 0xD01C setup: clear the two planes, transfer the menu batch, seed
    // the source menu-record metadata, and publish the initial label groups.
    std::function<void()> clear_menu_planes;
    std::function<void()> transfer_menu_asset_batch;
    std::function<void()> initialize_menu_record_metadata;
    std::function<void()> render_attribute_labels;
    std::function<void()> render_skill_labels;
    std::function<void()> render_base_attribute_skill_grids;

    // The active-runner context follows the static label/grid setup and
    // precedes the first detail panel and Attributes/Skills render.
    std::function<void()> refresh_active_runner_resource_metadata;
    std::function<void()> render_active_runner_hud_panel;
    std::function<void()> render_active_runner_name;
    std::function<void()> render_species_label;
    std::function<void()> render_archetype_label;
    std::function<void()> render_record_values;
    std::function<void(std::uint16_t)> render_detail_panel;
    std::function<AttributeSkillRenderResult()> render_attributes_and_skills;
};

// ROM 0x0D01C setup expressed as source-sized presentation/service steps.
// FFF116 starts negative in the source; a real render callback may publish a
// live selection result before the input loop begins.
void initialize_attributes_and_skills_menu(
    AttributesSkillsMenuState& state,
    const AttributesSkillsMenuInitializationCallbacks& callbacks = {});

enum class AttributesSkillsMenuInputEvent {
    waiting,
    selection_moved,
    upgrade_requested,
    upgrade_blocked,
    active_runner_selection_requested,
    exited_to_field,
};

enum class AttributesSkillsUnsupportedSelection {
    non_upgradeable_attribute,
    class_restricted_first_skill,
};

struct AttributesSkillsMenuInputFrame {
    bool move_left = false;  // source controller bit 10
    bool move_right = false; // source controller bit 11
    bool move_up = false;    // source controller bit 8
    bool move_down = false;  // source controller bit 9
    bool upgrade = false;    // source A/B bits 14/12
    bool active_runner_selection = false; // source C bit 13
    bool exit = false;       // source Start bit 15
};

struct AttributesSkillsMenuInputCallbacks {
    // Cursor changes and runner cycling draw the detail panel with D1=0x6000
    // before the caller's ordinary 0x4000/detail and D82A redraw pass.
    std::function<void(std::uint16_t)> render_detail_panel;
    std::function<void()> write_upgrade_grid;
    std::function<void(AttributesSkillsUnsupportedSelection)>
        render_unsupported_selection;
    std::function<void()> advance_active_runner_selection;
    std::function<void()> leave_to_field;
};

// ROM 0x0D01C/0x0D26A input priority as one loop iteration. Directional
// events are handled left, right, up, down before A/B upgrade, C runner
// cycling, and Start exit. The caller can cycle this contract in its own
// modern frame loop; no source input word or RAM marker is retained here.
AttributesSkillsMenuInputEvent apply_attributes_skills_menu_input(
    AttributesSkillsMenuState& state,
    const RunnerRecordFields& active_record,
    const AttributesSkillsMenuInputFrame& input,
    const AttributesSkillsMenuInputCallbacks& callbacks = {});

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
