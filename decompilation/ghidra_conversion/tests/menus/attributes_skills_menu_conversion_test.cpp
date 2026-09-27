#include "shadowrun_ghidra/menus/attributes_skills_menu_conversion.hpp"

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    RunnerRecordFields record{};
    record.archetype_index = 2U;
    record.species_index = 4U;
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

    RunnerRecordFields upgrade{};
    upgrade.species_index = 4U;
    upgrade.attributes[0] = 3U;
    upgrade.auxiliary[2] = 10U;
    const auto attribute_preview =
        preview_attribute_skill_upgrade(upgrade, {0U, 0U});
    if (!expect(attribute_preview.status ==
                    AttributeSkillUpgradeStatus::available &&
                attribute_preview.next_value == 4U &&
                attribute_preview.karma_cost == 4U &&
                apply_attribute_skill_upgrade(upgrade, {0U, 0U}, false) ==
                    AttributeSkillUpgradeResult::confirmation_required &&
                apply_attribute_skill_upgrade(upgrade, {0U, 0U}, true) ==
                    AttributeSkillUpgradeResult::upgraded &&
                upgrade.attributes[0] == 4U && upgrade.auxiliary[2] == 6U)) {
        return 9;
    }

    upgrade.attributes[6] = 0U;
    if (!expect(preview_attribute_skill_upgrade(upgrade, {1U, 2U}).status ==
                AttributeSkillUpgradeStatus::attribute_not_upgradeable)) {
        return 10;
    }

    RunnerRecordFields skill_upgrade{};
    skill_upgrade.species_index = 4U;
    skill_upgrade.archetype_index = 1U;
    skill_upgrade.skills[1] = 3U;
    skill_upgrade.skills[2] = 0U;
    skill_upgrade.auxiliary[2] = 6U;
    const auto skill_preview =
        preview_attribute_skill_upgrade(skill_upgrade, {0U, 6U});
    if (!expect(skill_preview.status ==
                    AttributeSkillUpgradeStatus::available &&
                skill_preview.next_value == 4U &&
                skill_preview.karma_cost == 6U &&
                apply_attribute_skill_upgrade(skill_upgrade, {0U, 6U}, true) ==
                    AttributeSkillUpgradeResult::upgraded &&
                skill_upgrade.skills[2] == 4U && skill_upgrade.auxiliary[2] == 0U)) {
        return 11;
    }

    skill_upgrade.archetype_index = 2U;
    if (!expect(preview_attribute_skill_upgrade(skill_upgrade, {0U, 4U}).status ==
                AttributeSkillUpgradeStatus::class_restricted)) {
        return 12;
    }

    RunnerRecordFields render_record{};
    render_record.species_index = 0U;
    render_record.attributes[0] = 3U;
    render_record.auxiliary[2] = 10U;
    std::vector<int> render_events;
    AttributeSkillRenderCallbacks render_callbacks{};
    render_callbacks.clear_upgrade_error_marker =
        [&] { render_events.push_back(1); };
    render_callbacks.write_attributes_skills_grid =
        [&] { render_events.push_back(2); };
    render_callbacks.render_attributes_skills_heading =
        [&] { render_events.push_back(3); };
    render_callbacks.render_upgrade_cost =
        [&](const std::uint8_t cost) {
            render_events.push_back(10 + cost);
        };
    render_callbacks.render_next_value =
        [&](const std::uint8_t value) {
            render_events.push_back(20 + value);
        };
    render_callbacks.render_invalid_upgrade_message =
        [&] { render_events.push_back(4); };
    render_callbacks.render_selected_label =
        [&](const AttributeSkillCell& cell) {
            render_events.push_back(30 + cell.label_index);
        };
    render_callbacks.render_selection_overlay_grid =
        [&] { render_events.push_back(5); };
    render_callbacks.render_karma_total =
        [&](const std::uint8_t karma) {
            render_events.push_back(40 + karma);
        };
    const auto rendered = render_attributes_and_skills(
        render_record, {0U, 0U}, true, render_callbacks);
    if (!expect(rendered.outcome ==
                    AttributeSkillRenderOutcome::rendered_upgrade_preview &&
                rendered.upgrade.karma_cost == 4U &&
                render_events == std::vector<int>{1, 2, 3, 14, 24, 30, 5,
                                                  50})) {
        return 13;
    }

    render_record.attributes[0] = 6U;
    render_events.clear();
    const auto at_limit = render_attributes_and_skills(
        render_record, {0U, 0U}, true, render_callbacks);
    if (!expect(at_limit.outcome ==
                    AttributeSkillRenderOutcome::invalid_upgrade_selection &&
                render_events == std::vector<int>{1, 2, 3, 4, 30, 5, 50})) {
        return 14;
    }

    render_events.clear();
    const auto unavailable = render_attributes_and_skills(
        render_record, {0U, 0U}, false, render_callbacks);
    if (!expect(unavailable.outcome ==
                    AttributeSkillRenderOutcome::selection_data_unavailable &&
                render_events == std::vector<int>{1, 2, 30, 5, 50})) {
        return 15;
    }

    AttributesSkillsMenuState menu_state{};
    std::vector<int> setup_events;
    AttributesSkillsMenuInitializationCallbacks setup_callbacks{};
    setup_callbacks.clear_menu_planes = [&] { setup_events.push_back(1); };
    setup_callbacks.transfer_menu_asset_batch =
        [&] { setup_events.push_back(2); };
    setup_callbacks.initialize_menu_record_metadata =
        [&] { setup_events.push_back(3); };
    setup_callbacks.render_attribute_labels =
        [&] { setup_events.push_back(4); };
    setup_callbacks.render_skill_labels =
        [&] { setup_events.push_back(5); };
    setup_callbacks.render_base_attribute_skill_grids =
        [&] { setup_events.push_back(6); };
    setup_callbacks.refresh_active_runner_resource_metadata =
        [&] { setup_events.push_back(7); };
    setup_callbacks.render_active_runner_hud_panel =
        [&] { setup_events.push_back(8); };
    setup_callbacks.render_active_runner_name =
        [&] { setup_events.push_back(9); };
    setup_callbacks.render_species_label = [&] { setup_events.push_back(10); };
    setup_callbacks.render_archetype_label =
        [&] { setup_events.push_back(11); };
    setup_callbacks.render_record_values =
        [&] { setup_events.push_back(12); };
    setup_callbacks.render_detail_panel =
        [&](const std::uint16_t command) {
            setup_events.push_back(command == 0x4000U ? 13 : 14);
        };
    setup_callbacks.render_attributes_and_skills = [&] {
        setup_events.push_back(15);
        AttributeSkillRenderResult result{};
        result.outcome =
            AttributeSkillRenderOutcome::rendered_upgrade_preview;
        return result;
    };
    initialize_attributes_and_skills_menu(menu_state, setup_callbacks);
    if (!expect(menu_state.open && menu_state.selection_data_available &&
                !menu_state.upgrade_error_marker &&
                setup_events == std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8,
                                                 9, 10, 11, 12, 13, 15})) {
        return 16;
    }

    RunnerRecordFields active_record{};
    active_record.archetype_index = 2U;
    std::vector<int> input_events;
    AttributesSkillsMenuInputCallbacks input_callbacks{};
    input_callbacks.render_detail_panel =
        [&](const std::uint16_t command) {
            input_events.push_back(command == 0x6000U ? 1 : 2);
        };
    input_callbacks.write_upgrade_grid = [&] { input_events.push_back(3); };
    input_callbacks.render_unsupported_selection =
        [&](const AttributesSkillsUnsupportedSelection reason) {
            input_events.push_back(
                reason == AttributesSkillsUnsupportedSelection::
                               non_upgradeable_attribute
                    ? 4
                    : 5);
        };
    input_callbacks.advance_active_runner_selection =
        [&] { input_events.push_back(6); };
    input_callbacks.leave_to_field = [&] { input_events.push_back(7); };

    menu_state.cursor = {1U, 9U};
    const auto left_priority = apply_attributes_skills_menu_input(
        menu_state, active_record,
        AttributesSkillsMenuInputFrame{true, true, false, true, true, false,
                                       false},
        input_callbacks);
    if (!expect(left_priority ==
                    AttributesSkillsMenuInputEvent::selection_moved &&
                menu_state.cursor.column == 0U && menu_state.cursor.row == 9U &&
                input_events == std::vector<int>{1})) {
        return 17;
    }

    menu_state.cursor = {1U, 2U};
    input_events.clear();
    const auto blocked_attribute = apply_attributes_skills_menu_input(
        menu_state, active_record,
        AttributesSkillsMenuInputFrame{false, false, false, false, true,
                                       false, false},
        input_callbacks);
    if (!expect(blocked_attribute ==
                    AttributesSkillsMenuInputEvent::upgrade_blocked &&
                input_events == std::vector<int>{3, 4})) {
        return 18;
    }

    menu_state.cursor = {0U, 4U};
    input_events.clear();
    const auto blocked_skill = apply_attributes_skills_menu_input(
        menu_state, active_record,
        AttributesSkillsMenuInputFrame{false, false, false, false, true,
                                       false, false},
        input_callbacks);
    if (!expect(blocked_skill ==
                    AttributesSkillsMenuInputEvent::upgrade_blocked &&
                input_events == std::vector<int>{3, 5})) {
        return 19;
    }

    active_record.archetype_index = 1U;
    input_events.clear();
    const auto upgrade_event = apply_attributes_skills_menu_input(
        menu_state, active_record,
        AttributesSkillsMenuInputFrame{false, false, false, false, true,
                                       false, false},
        input_callbacks);
    if (!expect(upgrade_event ==
                    AttributesSkillsMenuInputEvent::upgrade_requested &&
                input_events == std::vector<int>{3})) {
        return 20;
    }

    menu_state.cursor = {1U, 4U};
    input_events.clear();
    const auto cycle = apply_attributes_skills_menu_input(
        menu_state, active_record,
        AttributesSkillsMenuInputFrame{false, false, false, false, false, true,
                                       true},
        input_callbacks);
    if (!expect(cycle ==
                    AttributesSkillsMenuInputEvent::
                        active_runner_selection_requested &&
                menu_state.cursor.column == 0U &&
                input_events == std::vector<int>{1, 6})) {
        return 21;
    }

    input_events.clear();
    const auto exit = apply_attributes_skills_menu_input(
        menu_state, active_record,
        AttributesSkillsMenuInputFrame{false, false, false, false, false, false,
                                       true},
        input_callbacks);
    if (!expect(exit == AttributesSkillsMenuInputEvent::exited_to_field &&
                !menu_state.open && input_events == std::vector<int>{7})) {
        return 22;
    }

    return 0;
}
