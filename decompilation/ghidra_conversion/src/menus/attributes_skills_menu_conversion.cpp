#include "shadowrun_ghidra/menus/attributes_skills_menu_conversion.hpp"

#include <algorithm>
#include <cstddef>

namespace shadowrun::ghidra_conversion {
namespace {

constexpr std::array<std::array<std::uint8_t, 6U>, 5U>
    kAttributeMaximums{{
        {{6U, 6U, 6U, 6U, 6U, 6U}},
        {{6U, 7U, 6U, 8U, 6U, 6U}},
        {{7U, 5U, 8U, 6U, 6U, 7U}},
        {{9U, 6U, 8U, 5U, 5U, 6U}},
        {{11U, 5U, 10U, 4U, 4U, 5U}},
    }};

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

AttributeSkillRenderResult render_attributes_and_skills(
    const RunnerRecordFields& record, const AttributeSkillCursor cursor,
    const bool selection_data_available,
    const AttributeSkillRenderCallbacks& callbacks) {
    AttributeSkillRenderResult result{};
    result.karma_available = record.auxiliary[2U];

    if (callbacks.clear_upgrade_error_marker) {
        callbacks.clear_upgrade_error_marker();
    }
    if (callbacks.write_attributes_skills_grid) {
        callbacks.write_attributes_skills_grid();
    }

    const auto grid = read_attributes_and_skills_grid(record, true);
    const auto clamped_column = std::min<std::size_t>(
        cursor.column, kAttributesSkillsColumnCount - 1U);
    const auto clamped_row = std::min<std::size_t>(
        cursor.row, kAttributesSkillsRowCount - 1U);
    result.selected_cell = grid[clamped_column][clamped_row];

    if (!selection_data_available) {
        result.outcome =
            AttributeSkillRenderOutcome::selection_data_unavailable;
    } else {
        const bool attribute_outside_upgrade_table =
            result.selected_cell.section == AttributeSkillSection::attribute &&
            result.selected_cell.value_index >= 6U;
        const bool restricted_first_skill =
            result.selected_cell.section == AttributeSkillSection::skill &&
            result.selected_cell.value_index == 0U &&
            record.archetype_index >= 2U;

        const bool source_suppresses_error_message =
            attribute_outside_upgrade_table || restricted_first_skill;
        if (source_suppresses_error_message) {
            result.outcome =
                AttributeSkillRenderOutcome::invalid_upgrade_selection;
        } else {
            if (callbacks.render_attributes_skills_heading) {
                callbacks.render_attributes_skills_heading();
            }
            result.upgrade = preview_attribute_skill_upgrade(record, cursor);
            if (result.upgrade.status ==
                AttributeSkillUpgradeStatus::available) {
                result.outcome =
                    AttributeSkillRenderOutcome::rendered_upgrade_preview;
                if (callbacks.render_upgrade_cost) {
                    callbacks.render_upgrade_cost(
                        static_cast<std::uint8_t>(result.upgrade.karma_cost));
                }
                if (callbacks.render_next_value) {
                    callbacks.render_next_value(result.upgrade.next_value);
                }
            } else {
                result.outcome =
                    AttributeSkillRenderOutcome::invalid_upgrade_selection;
            }
        }

        if (!source_suppresses_error_message &&
            result.outcome ==
            AttributeSkillRenderOutcome::invalid_upgrade_selection) {
            if (callbacks.render_invalid_upgrade_message) {
                callbacks.render_invalid_upgrade_message();
            }
        }
    }

    if (callbacks.render_selected_label) {
        callbacks.render_selected_label(result.selected_cell);
    }
    if (callbacks.render_selection_overlay_grid) {
        callbacks.render_selection_overlay_grid();
    }
    if (callbacks.render_karma_total) {
        callbacks.render_karma_total(result.karma_available);
    }
    return result;
}

void initialize_attributes_and_skills_menu(
    AttributesSkillsMenuState& state,
    const AttributesSkillsMenuInitializationCallbacks& callbacks) {
    state.cursor = {};
    state.selection_data_available = false;
    state.upgrade_error_marker = false;
    state.open = true;

    if (callbacks.clear_menu_planes) {
        callbacks.clear_menu_planes();
    }
    if (callbacks.transfer_menu_asset_batch) {
        callbacks.transfer_menu_asset_batch();
    }
    if (callbacks.initialize_menu_record_metadata) {
        callbacks.initialize_menu_record_metadata();
    }
    if (callbacks.render_attribute_labels) {
        callbacks.render_attribute_labels();
    }
    if (callbacks.render_skill_labels) {
        callbacks.render_skill_labels();
    }
    if (callbacks.render_base_attribute_skill_grids) {
        callbacks.render_base_attribute_skill_grids();
    }
    if (callbacks.refresh_active_runner_resource_metadata) {
        callbacks.refresh_active_runner_resource_metadata();
    }
    if (callbacks.render_active_runner_hud_panel) {
        callbacks.render_active_runner_hud_panel();
    }
    if (callbacks.render_active_runner_name) {
        callbacks.render_active_runner_name();
    }
    if (callbacks.render_species_label) {
        callbacks.render_species_label();
    }
    if (callbacks.render_archetype_label) {
        callbacks.render_archetype_label();
    }
    if (callbacks.render_record_values) {
        callbacks.render_record_values();
    }
    if (callbacks.render_detail_panel) {
        callbacks.render_detail_panel(0x4000U);
    }
    if (callbacks.render_attributes_and_skills) {
        const auto result = callbacks.render_attributes_and_skills();
        state.selection_data_available =
            result.outcome !=
            AttributeSkillRenderOutcome::selection_data_unavailable;
        state.upgrade_error_marker =
            result.outcome ==
            AttributeSkillRenderOutcome::invalid_upgrade_selection;
    }
}

AttributesSkillsMenuInputEvent apply_attributes_skills_menu_input(
    AttributesSkillsMenuState& state,
    const RunnerRecordFields& active_record,
    const AttributesSkillsMenuInputFrame& input,
    const AttributesSkillsMenuInputCallbacks& callbacks) {
    if (!state.open) {
        return AttributesSkillsMenuInputEvent::exited_to_field;
    }

    // The source uses a branch-and-redraw after each direction, so later
    // directions and action bits in the same sample are not considered.
    if (input.move_left) {
        if (callbacks.render_detail_panel) {
            callbacks.render_detail_panel(0x6000U);
        }
        state.cursor.column = 0U;
        return AttributesSkillsMenuInputEvent::selection_moved;
    }
    if (input.move_right) {
        if (callbacks.render_detail_panel) {
            callbacks.render_detail_panel(0x6000U);
        }
        state.cursor.column = 1U;
        return AttributesSkillsMenuInputEvent::selection_moved;
    }
    if (input.move_up) {
        if (callbacks.render_detail_panel) {
            callbacks.render_detail_panel(0x6000U);
        }
        if (state.cursor.row > 0U) {
            --state.cursor.row;
        }
        return AttributesSkillsMenuInputEvent::selection_moved;
    }
    if (input.move_down) {
        if (callbacks.render_detail_panel) {
            callbacks.render_detail_panel(0x6000U);
        }
        if (state.cursor.row < 9U) {
            ++state.cursor.row;
        }
        return AttributesSkillsMenuInputEvent::selection_moved;
    }

    // A and B share the source upgrade path. A negative FFF116 marker exits
    // the screen, while a negative FFF0B3 marker means D82A already rendered
    // the rejection and the loop simply waits for the next sample.
    if (input.upgrade) {
        if (!state.selection_data_available) {
            state.open = false;
            if (callbacks.leave_to_field) {
                callbacks.leave_to_field();
            }
            return AttributesSkillsMenuInputEvent::exited_to_field;
        }
        if (state.upgrade_error_marker) {
            return AttributesSkillsMenuInputEvent::upgrade_blocked;
        }
        if (callbacks.write_upgrade_grid) {
            callbacks.write_upgrade_grid();
        }
        if (state.cursor.column == 1U &&
            (state.cursor.row == 2U || state.cursor.row == 3U)) {
            if (callbacks.render_unsupported_selection) {
                callbacks.render_unsupported_selection(
                    AttributesSkillsUnsupportedSelection::
                        non_upgradeable_attribute);
            }
            return AttributesSkillsMenuInputEvent::upgrade_blocked;
        }
        if (state.cursor.column == 0U && state.cursor.row == 4U &&
            active_record.archetype_index >= 2U) {
            if (callbacks.render_unsupported_selection) {
                callbacks.render_unsupported_selection(
                    AttributesSkillsUnsupportedSelection::
                        class_restricted_first_skill);
            }
            return AttributesSkillsMenuInputEvent::upgrade_blocked;
        }
        return AttributesSkillsMenuInputEvent::upgrade_requested;
    }

    // C takes priority over Start in the source's final branch. The detail
    // panel is prepared before the active-runner selection service.
    if (input.active_runner_selection) {
        if (callbacks.render_detail_panel) {
            callbacks.render_detail_panel(0x6000U);
        }
        state.cursor.column = 0U;
        if (callbacks.advance_active_runner_selection) {
            callbacks.advance_active_runner_selection();
        }
        return AttributesSkillsMenuInputEvent::
            active_runner_selection_requested;
    }

    if (input.exit) {
        state.open = false;
        if (callbacks.leave_to_field) {
            callbacks.leave_to_field();
        }
        return AttributesSkillsMenuInputEvent::exited_to_field;
    }
    return AttributesSkillsMenuInputEvent::waiting;
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
    if (left) {
        cursor.column = 0U;
    } else if (right) {
        cursor.column = 1U;
    }

    if (up && cursor.row > 0U) {
        --cursor.row;
    } else if (down && cursor.row < 9U) {
        ++cursor.row;
    }
    return cursor;
}

AttributeSkillUpgradePreview preview_attribute_skill_upgrade(
    const RunnerRecordFields& record, const AttributeSkillCursor cursor) {
    AttributeSkillUpgradePreview preview{};
    if (cursor.column >= kAttributesSkillsColumnCount ||
        cursor.row >= kAttributesSkillsRowCount) {
        return preview;
    }

    const auto source = source_cell(cursor.column, cursor.row);
    preview.section = source.section;
    preview.value_index = source.value_index;
    preview.record_offset = source.record_offset;
    if (source.section == AttributeSkillSection::attribute) {
        preview.current_value = record.attributes[source.value_index];
        if (source.value_index >= 6U) {
            preview.status =
                AttributeSkillUpgradeStatus::attribute_not_upgradeable;
            return preview;
        }
        if (record.species_index >= kAttributeMaximums.size()) {
            preview.status = AttributeSkillUpgradeStatus::unknown_species;
            return preview;
        }
        const auto next = static_cast<unsigned>(preview.current_value) + 1U;
        if (next > kAttributeMaximums[record.species_index]
                              [source.value_index]) {
            preview.status = AttributeSkillUpgradeStatus::at_limit;
            return preview;
        }
        preview.next_value = static_cast<std::uint8_t>(next);
        // D82A leaves D1 as next for attributes; D3EA subtracts FFF0B8
        // directly from +0x8D. The doubled branch belongs to the skill path.
        preview.karma_cost = static_cast<std::uint16_t>(next);
        preview.status = AttributeSkillUpgradeStatus::available;
        return preview;
    }

    preview.current_value = record.skills[source.value_index];
    if (source.value_index == 0U && record.archetype_index >= 2U) {
        preview.status = AttributeSkillUpgradeStatus::class_restricted;
        return preview;
    }

    // The left-column middle skill branch in 0xD958 uses the second skill's
    // current value when the selected skill is zero. Preserve that source
    // dependency instead of reducing every skill to one generic formula.
    // 0xD920 tests the selected skill's own value + 1 against 0x0D before
    // the Firearms substitution below.
    if (static_cast<unsigned>(preview.current_value) + 1U >= 0x0DU) {
        preview.status = AttributeSkillUpgradeStatus::at_limit;
        return preview;
    }
    auto next_base = static_cast<unsigned>(preview.current_value);
    const bool reduced_skill_cost = cursor.column == 0U &&
                                    source.value_index >= 2U &&
                                    source.value_index <= 4U;
    if (reduced_skill_cost && preview.current_value == 0U) {
        next_base = record.skills[1U];
    }
    const auto next = next_base + 1U;
    preview.next_value = static_cast<std::uint8_t>(next);
    if (reduced_skill_cost) {
        preview.karma_cost = static_cast<std::uint16_t>(
            (next * 15U + 9U) / 10U);
    } else {
        preview.karma_cost = static_cast<std::uint16_t>(next * 2U);
    }
    preview.status = AttributeSkillUpgradeStatus::available;
    return preview;
}

AttributeSkillUpgradeResult apply_attribute_skill_upgrade(
    RunnerRecordFields& record, const AttributeSkillCursor cursor,
    const bool confirmed) {
    return apply_attribute_skill_upgrade(
        record, preview_attribute_skill_upgrade(record, cursor), confirmed);
}

AttributeSkillUpgradeResult apply_attribute_skill_upgrade(
    RunnerRecordFields& record, const AttributeSkillUpgradePreview& preview,
    const bool confirmed) {
    switch (preview.status) {
    case AttributeSkillUpgradeStatus::invalid_selection:
        return AttributeSkillUpgradeResult::invalid_selection;
    case AttributeSkillUpgradeStatus::attribute_not_upgradeable:
        return AttributeSkillUpgradeResult::attribute_not_upgradeable;
    case AttributeSkillUpgradeStatus::unknown_species:
        return AttributeSkillUpgradeResult::unknown_species;
    case AttributeSkillUpgradeStatus::class_restricted:
        return AttributeSkillUpgradeResult::class_restricted;
    case AttributeSkillUpgradeStatus::at_limit:
        return AttributeSkillUpgradeResult::at_limit;
    case AttributeSkillUpgradeStatus::available:
        break;
    }

    if (record.auxiliary[2U] < preview.karma_cost) {
        return AttributeSkillUpgradeResult::insufficient_karma;
    }
    if (!confirmed) {
        return AttributeSkillUpgradeResult::confirmation_required;
    }

    record.auxiliary[2U] = static_cast<std::uint8_t>(
        record.auxiliary[2U] - preview.karma_cost);
    if (preview.section == AttributeSkillSection::attribute) {
        record.attributes[preview.value_index] = preview.next_value;
    } else {
        record.skills[preview.value_index] = preview.next_value;
    }
    return AttributeSkillUpgradeResult::upgraded;
}

} // namespace shadowrun::ghidra_conversion
