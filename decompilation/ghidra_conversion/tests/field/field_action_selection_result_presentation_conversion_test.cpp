#include "shadowrun_ghidra/field/field_action_selection_result_presentation_conversion.hpp"

#include <array>
#include <cstdint>
#include <utility>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    int grids = 0;
    int glyph_triples = 0;
    int text_passes = 0;
    std::vector<MenuResourceRecord> selected_resources;
    std::vector<FieldActionSelectionDetailGlyphTable> selected_negative_tables;
    std::vector<std::pair<FieldActionSelectionActionRecordTable, std::int16_t>>
        selected_action_tables;
    const FieldActionSelectionPresentationCallbacks callbacks{
        [&] { ++grids; },
        [&] { ++glyph_triples; },
        [&] { ++text_passes; },
        [&](const MenuResourceRecord& resource) {
            selected_resources.push_back(resource);
        },
        [&](const FieldActionSelectionDetailGlyphTable table, const std::uint8_t) {
            selected_negative_tables.push_back(table);
        },
        [&](const FieldActionSelectionActionRecordTable table, const std::int16_t selector) {
            selected_action_tables.emplace_back(table, selector);
        },
    };

    const auto general_resource = select_menu_resource(4, 5);
    const auto mode_six_resource = select_menu_resource(6, 5);
    if (!expect(general_resource.family ==
                    MenuResourceFamily::general_menu_records &&
                general_resource.record_offset == 0x50U &&
                mode_six_resource.family ==
                    MenuResourceFamily::menu_mode_six_records &&
                mode_six_resource.record_offset == 0x50U &&
                select_menu_resource(4, -1).record_offset == 0x0FF0U)) {
        return 1;
    }
    const auto selected_entry = select_menu_resource_entry(6, 5, 3);
    if (!expect(selected_entry.resource.family ==
                    MenuResourceFamily::menu_mode_six_records &&
                selected_entry.resource.record_offset == 0x50U &&
                selected_entry.word_offset == 0x06U)) {
        return 11;
    }

    const auto grid_record = plan_field_action_selection_record('E');
    const auto low_palette_record = plan_field_action_selection_record(0x0EU);
    const auto high_palette_record = plan_field_action_selection_record(0x0FU);
    const auto blank_inventory = plan_field_action_selection_inventory_glyph(0);
    const auto item_inventory = plan_field_action_selection_inventory_glyph(0x10U);
    const auto cursor_three_plan =
        plan_field_action_selection_positive_grid(6, 2, 3, 2);
    const auto cursor_five_plan =
        plan_field_action_selection_positive_grid(5, 2, 5, 2);
    const auto cursor_other_plan =
        plan_field_action_selection_positive_grid(5, 2, 4, 2);
    const auto resource_plan =
        plan_field_action_selection_positive_grid(5, 2, 1, 2);
    const auto blank_resource_row = plan_field_action_selection_resource_grid_row(0x80U);
    const auto indexed_resource_row = plan_field_action_selection_resource_grid_row(7U);
    const auto blank_positive_row =
        plan_field_action_selection_positive_resource_row(0x80U);
    const auto indexed_positive_row = plan_field_action_selection_positive_resource_row(7U);
    if (!expect(grid_record.writes_grid &&
                low_palette_record.glyph_palette ==
                    FieldActionSelectionGlyphPalettePlane::low_0x2000 &&
                !low_palette_record.writes_grid &&
                high_palette_record.glyph_palette ==
                    FieldActionSelectionGlyphPalettePlane::high_0x6000 &&
                blank_inventory.uses_blank_asset &&
                !item_inventory.uses_blank_asset &&
                item_inventory.item_definition_index == 0x0FU &&
                item_inventory.glyph_palette ==
                    FieldActionSelectionGlyphPalettePlane::high_0x6000 &&
                cursor_three_plan.kind ==
                    FieldActionSelectionPositiveGridPlanKind::field_resource_record_purchase_rows &&
                cursor_three_plan.resource_table_family ==
                    FieldActionSelectionPositiveResourceTableFamily::mode_six_resource_records &&
                cursor_three_plan.resource_table_offset == 0U &&
                cursor_three_plan.first_grid_count == 8U &&
                cursor_five_plan.kind ==
                    FieldActionSelectionPositiveGridPlanKind::field_quantity_purchase_split &&
                cursor_five_plan.first_grid_count == 5U &&
                cursor_five_plan.second_grid_count == 3U &&
                cursor_other_plan.kind ==
                    FieldActionSelectionPositiveGridPlanKind::field_resource_purchase_split &&
                cursor_other_plan.first_grid_count == 4U &&
                cursor_other_plan.second_grid_count == 4U &&
                resource_plan.kind ==
                    FieldActionSelectionPositiveGridPlanKind::matrix_program_resource_description &&
                resource_plan.selects_menu_resource &&
                resource_plan.selected_resource.record_offset == 0x20U &&
                blank_resource_row.uses_blank_grid &&
                !indexed_resource_row.uses_blank_grid &&
                indexed_resource_row.description_record_offset == 0x54U &&
                blank_positive_row.uses_blank_grid &&
                !indexed_positive_row.uses_blank_grid)) {
        return 12;
    }

    int positive_plan_grids = 0;
    FieldActionSelectionPositiveGridPlan observed_positive_plan{};
    FieldActionSelectionPresentationCallbacks positive_callbacks{};
    positive_callbacks.write_grid = [&] { ++positive_plan_grids; };
    positive_callbacks.select_positive_grid_plan =
        [&](const FieldActionSelectionPositiveGridPlan& plan) {
            observed_positive_plan = plan;
        };
    FieldActionSelectionPresentationState positive_cursor_three{};
    positive_cursor_three.menu_result = 2;
    positive_cursor_three.menu_mode = 6;
    positive_cursor_three.selected_action = 2;
    positive_cursor_three.action_cursor = 3;
    render_field_action_selection_presentation(positive_cursor_three,
                                             positive_callbacks);
    if (!expect(observed_positive_plan.kind ==
                    FieldActionSelectionPositiveGridPlanKind::field_resource_record_purchase_rows &&
                observed_positive_plan.resource_table_offset == 0U &&
                positive_plan_grids == 8)) {
        return 14;
    }

    std::vector<FieldActionSelectionPositiveResourceRowPlan> positive_rows;
    FieldActionSelectionPresentationCallbacks positive_row_callbacks{};
    positive_row_callbacks.write_positive_resource_row =
        [&](const std::size_t, const FieldActionSelectionPositiveResourceRowPlan& row) {
            positive_rows.push_back(row);
        };
    positive_cursor_three.positive_resource_markers[0] = 0x80U;
    positive_cursor_three.positive_resource_markers[1] = 7U;
    render_field_action_selection_presentation(positive_cursor_three,
                                             positive_row_callbacks);
    if (!expect(positive_rows.size() == 8U &&
                positive_rows[0].uses_blank_grid &&
                !positive_rows[1].uses_blank_grid)) {
        return 15;
    }

    std::vector<FieldActionSelectionResourceGridRowPlan> resource_rows;
    FieldActionSelectionPresentationCallbacks resource_row_callbacks{};
    resource_row_callbacks.write_resource_grid_row =
        [&](const std::size_t, const FieldActionSelectionResourceGridRowPlan& row) {
            resource_rows.push_back(row);
        };
    FieldActionSelectionPresentationState matrix_program_resource_description{};
    matrix_program_resource_description.menu_result = 2;
    matrix_program_resource_description.menu_mode = 5;
    matrix_program_resource_description.action_cursor = 1;
    matrix_program_resource_description.resource_grid_markers[0] = 0x80U;
    matrix_program_resource_description.resource_grid_markers[1] = 7U;
    render_field_action_selection_presentation(matrix_program_resource_description,
                                             resource_row_callbacks);
    if (!expect(resource_rows.size() == 8U &&
                resource_rows[0].uses_blank_grid &&
                resource_rows[1].description_record_offset == 0x54U)) {
        return 16;
    }

    std::vector<std::size_t> inventory_slots;
    std::vector<FieldActionSelectionInventoryGlyphPlan> inventory_plans;
    int inventory_text_passes = 0;
    FieldActionSelectionPresentationCallbacks inventory_callbacks{};
    inventory_callbacks.transfer_inventory_glyph =
        [&](const std::size_t slot, const FieldActionSelectionInventoryGlyphPlan& plan) {
            inventory_slots.push_back(slot);
            inventory_plans.push_back(plan);
        };
    inventory_callbacks.render_encoded_text = [&] { ++inventory_text_passes; };
    render_field_action_selection_inventory_glyph_row(
        std::array<std::uint8_t, 8>{0U, 1U, 0x10U, 0x80U, 0U, 0U, 0U, 0U},
        4,
        inventory_callbacks);
    if (!expect(inventory_slots.size() == 8U &&
                inventory_plans[0].uses_blank_asset &&
                !inventory_plans[1].uses_blank_asset &&
                inventory_plans[1].item_definition_index == 0U &&
                inventory_plans[2].item_definition_index == 0x0FU &&
                inventory_plans[3].item_definition_index == 0x7FU &&
                inventory_text_passes == 1)) {
        return 13;
    }

    std::array<FieldActionSelectionGlyphRecord, 8> records{};
    records[0].marker = 'E';
    records[1].marker = -1;
    records[2].marker = 'X';
    int record_grids = 0;
    int record_glyphs = 0;
    FieldActionSelectionPresentationCallbacks record_callbacks{
        [&] { ++record_grids; },
        [&] { ++record_glyphs; },
        {},
        {},
        {},
        {},
    };
    std::vector<FieldActionSelectionRecordPresentationPlan> record_glyph_plans;
    std::vector<FieldActionSelectionRecordPresentationPlan> record_grid_plans;
    record_callbacks.transfer_record_glyph =
        [&](const std::size_t, const FieldActionSelectionRecordPresentationPlan& plan) {
            ++record_glyphs;
            record_glyph_plans.push_back(plan);
        };
    record_callbacks.write_record_grid =
        [&](const std::size_t, const FieldActionSelectionRecordPresentationPlan& plan) {
            ++record_grids;
            record_grid_plans.push_back(plan);
        };
    render_alternating_field_action_selection_records(records, record_callbacks);
    if (!expect(record_grids == 1 && record_glyphs == 7 &&
                record_grid_plans.front().writes_grid &&
                record_grid_plans.front().marker ==
                    static_cast<std::uint8_t>('E') &&
                record_glyph_plans[0].marker == 0xFFU &&
                record_glyph_plans[0].glyph_palette ==
                    FieldActionSelectionGlyphPalettePlane::high_0x6000)) {
        return 2;
    }

    FieldActionSelectionPresentationState negative{};
    negative.menu_result = -1;
    negative.action_cursor = 3;
    render_field_action_selection_presentation(negative, callbacks);
    if (!expect(negative.selected_action == 3 && grids == 8 &&
                glyph_triples == 0)) {
        return 3;
    }

    FieldActionSelectionPresentationState negative_table{};
    negative_table.menu_result = -1;
    negative_table.action_cursor = 1;
    negative_table.action_detail_glyph_records[1][0].marker = 'E';
    render_field_action_selection_presentation(negative_table, callbacks);
    if (!expect(selected_negative_tables.size() == 1U &&
                selected_negative_tables.back() ==
                    FieldActionSelectionDetailGlyphTable::action_detail_cursor_one &&
                grids == 9 && glyph_triples == 7)) {
        return 31;
    }

    FieldActionSelectionPresentationState negative_glyphs{};
    negative_glyphs.menu_result = -1;
    negative_glyphs.action_cursor = 4;
    render_field_action_selection_presentation(negative_glyphs, callbacks);
    if (!expect(glyph_triples == 15 && text_passes == 1)) {
        return 4;
    }

    FieldActionSelectionPresentationState negative_high_cursor{};
    negative_high_cursor.menu_result = -1;
    negative_high_cursor.action_cursor = static_cast<std::int8_t>(0x80U);
    render_field_action_selection_presentation(negative_high_cursor, callbacks);
    if (!expect(glyph_triples == 23 && text_passes == 1)) {
        return 32;
    }

    FieldActionSelectionPresentationState result_one{};
    result_one.menu_result = 1;
    result_one.action_cursor = 7;
    render_field_action_selection_presentation(result_one, callbacks);
    if (!expect(grids == 17)) {
        return 5;
    }

    FieldActionSelectionPresentationState result_two{};
    result_two.menu_result = 2;
    result_two.menu_mode = 6;
    result_two.action_cursor = 2;
    render_field_action_selection_presentation(result_two, callbacks);
    if (!expect(grids == 25 && selected_resources.size() == 1U &&
                selected_resources.back().family ==
                    MenuResourceFamily::menu_mode_six_records &&
                selected_resources.back().record_offset == 0x20U)) {
        return 6;
    }

    result_two.action_cursor = 5;
    render_field_action_selection_presentation(result_two, callbacks);
    if (!expect(grids == 33)) {
        return 7;
    }
    result_two.action_cursor = 4;
    render_field_action_selection_presentation(result_two, callbacks);
    if (!expect(grids == 41)) {
        return 8;
    }

    FieldActionSelectionPresentationState positive_high_cursor{};
    positive_high_cursor.menu_result = 2;
    positive_high_cursor.menu_mode = 6;
    positive_high_cursor.action_cursor = static_cast<std::int8_t>(0x80U);
    render_field_action_selection_presentation(positive_high_cursor, callbacks);
    if (!expect(grids == 49 && selected_resources.size() == 1U)) {
        return 33;
    }

    FieldActionSelectionPresentationState normal{};
    normal.menu_result = 0;
    normal.menu_mode = 5;
    normal.action_cursor = 0;
    render_field_action_selection_presentation(normal, callbacks);
    if (!expect(selected_action_tables.size() == 1U &&
                selected_action_tables.back().first ==
                    FieldActionSelectionActionRecordTable::dynamic_menu_mode &&
                selected_action_tables.back().second == 2 &&
                glyph_triples == 31 && text_passes == 1)) {
        return 9;
    }
    normal.action_cursor = 1;
    render_field_action_selection_presentation(normal, callbacks);
    if (!expect(selected_action_tables.size() == 2U &&
                selected_action_tables.back().first ==
                    FieldActionSelectionActionRecordTable::fixed_selection_one &&
                selected_action_tables.back().second == 2)) {
        return 35;
    }
    normal.action_cursor = 3;
    render_field_action_selection_presentation(normal, callbacks);
    if (!expect(glyph_triples == 47 && text_passes == 1)) {
        return 10;
    }

    FieldActionSelectionPresentationState dynamic_action_table{};
    dynamic_action_table.menu_result = 0;
    dynamic_action_table.menu_mode = 5;
    dynamic_action_table.action_cursor = -1;
    render_field_action_selection_presentation(dynamic_action_table, callbacks);
    if (!expect(selected_action_tables.size() == 3U &&
                selected_action_tables.back().first ==
                    FieldActionSelectionActionRecordTable::dynamic_menu_mode &&
                selected_action_tables.back().second == 2 &&
                glyph_triples == 55)) {
        return 11;
    }

    FieldActionSelectionPresentationState high_zero{};
    high_zero.menu_result = 0;
    high_zero.menu_mode = 5;
    high_zero.action_cursor = static_cast<std::int8_t>(0x80U);
    render_field_action_selection_presentation(high_zero, callbacks);
    if (!expect(selected_action_tables.size() == 4U &&
                selected_action_tables.back().first ==
                    FieldActionSelectionActionRecordTable::fixed_low_menu &&
                selected_action_tables.back().second == 2 &&
                glyph_triples == 63)) {
        return 34;
    }

    return 0;
}
