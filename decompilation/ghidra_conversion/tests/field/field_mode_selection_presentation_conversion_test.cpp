#include "shadowrun_ghidra/field/field_mode_selection_presentation_conversion.hpp"

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
    std::vector<FieldModeNegativeGlyphTable> selected_negative_tables;
    std::vector<std::pair<FieldModeActionRecordTable, std::int16_t>>
        selected_action_tables;
    const FieldModeSelectionPresentationCallbacks callbacks{
        [&] { ++grids; },
        [&] { ++glyph_triples; },
        [&] { ++text_passes; },
        [&](const MenuResourceRecord& resource) {
            selected_resources.push_back(resource);
        },
        [&](const FieldModeNegativeGlyphTable table, const std::uint8_t) {
            selected_negative_tables.push_back(table);
        },
        [&](const FieldModeActionRecordTable table, const std::int16_t selector) {
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

    std::array<FieldModeSelectionGlyphRecord, 8> records{};
    records[0].marker = 'E';
    records[1].marker = -1;
    records[2].marker = 'X';
    int record_grids = 0;
    int record_glyphs = 0;
    const FieldModeSelectionPresentationCallbacks record_callbacks{
        [&] { ++record_grids; },
        [&] { ++record_glyphs; },
        {},
        {},
        {},
        {},
    };
    render_alternating_field_mode_records(records, record_callbacks);
    if (!expect(record_grids == 1 && record_glyphs == 7)) {
        return 2;
    }

    FieldModeSelectionPresentationState negative{};
    negative.mode_result = -1;
    negative.primary_cursor = 3;
    render_field_mode_selection_presentation(negative, callbacks);
    if (!expect(negative.saved_selection == 3 && grids == 8 &&
                glyph_triples == 0)) {
        return 3;
    }

    FieldModeSelectionPresentationState negative_table{};
    negative_table.mode_result = -1;
    negative_table.primary_cursor = 1;
    negative_table.negative_glyph_records[1][0].marker = 'E';
    render_field_mode_selection_presentation(negative_table, callbacks);
    if (!expect(selected_negative_tables.size() == 1U &&
                selected_negative_tables.back() ==
                    FieldModeNegativeGlyphTable::cursor_one &&
                grids == 9 && glyph_triples == 7)) {
        return 31;
    }

    FieldModeSelectionPresentationState negative_glyphs{};
    negative_glyphs.mode_result = -1;
    negative_glyphs.primary_cursor = 4;
    render_field_mode_selection_presentation(negative_glyphs, callbacks);
    if (!expect(glyph_triples == 15 && text_passes == 1)) {
        return 4;
    }

    FieldModeSelectionPresentationState negative_high_cursor{};
    negative_high_cursor.mode_result = -1;
    negative_high_cursor.primary_cursor = static_cast<std::int8_t>(0x80U);
    render_field_mode_selection_presentation(negative_high_cursor, callbacks);
    if (!expect(glyph_triples == 23 && text_passes == 1)) {
        return 32;
    }

    FieldModeSelectionPresentationState result_one{};
    result_one.mode_result = 1;
    result_one.primary_cursor = 7;
    render_field_mode_selection_presentation(result_one, callbacks);
    if (!expect(grids == 17)) {
        return 5;
    }

    FieldModeSelectionPresentationState result_two{};
    result_two.mode_result = 2;
    result_two.menu_mode = 6;
    result_two.primary_cursor = 2;
    render_field_mode_selection_presentation(result_two, callbacks);
    if (!expect(grids == 25 && selected_resources.size() == 1U &&
                selected_resources.back().family ==
                    MenuResourceFamily::menu_mode_six_records &&
                selected_resources.back().record_offset == 0x20U)) {
        return 6;
    }

    result_two.primary_cursor = 5;
    render_field_mode_selection_presentation(result_two, callbacks);
    if (!expect(grids == 33)) {
        return 7;
    }
    result_two.primary_cursor = 4;
    render_field_mode_selection_presentation(result_two, callbacks);
    if (!expect(grids == 41)) {
        return 8;
    }

    FieldModeSelectionPresentationState positive_high_cursor{};
    positive_high_cursor.mode_result = 2;
    positive_high_cursor.menu_mode = 6;
    positive_high_cursor.primary_cursor = static_cast<std::int8_t>(0x80U);
    render_field_mode_selection_presentation(positive_high_cursor, callbacks);
    if (!expect(grids == 49 && selected_resources.size() == 1U)) {
        return 33;
    }

    FieldModeSelectionPresentationState normal{};
    normal.mode_result = 0;
    normal.menu_mode = 5;
    normal.primary_cursor = 0;
    render_field_mode_selection_presentation(normal, callbacks);
    if (!expect(selected_action_tables.size() == 1U &&
                selected_action_tables.back().first ==
                    FieldModeActionRecordTable::dynamic_menu_mode &&
                selected_action_tables.back().second == 2 &&
                glyph_triples == 31 && text_passes == 1)) {
        return 9;
    }
    normal.primary_cursor = 3;
    render_field_mode_selection_presentation(normal, callbacks);
    if (!expect(glyph_triples == 39 && text_passes == 1)) {
        return 10;
    }

    FieldModeSelectionPresentationState dynamic_action_table{};
    dynamic_action_table.mode_result = 0;
    dynamic_action_table.menu_mode = 5;
    dynamic_action_table.primary_cursor = -1;
    render_field_mode_selection_presentation(dynamic_action_table, callbacks);
    if (!expect(selected_action_tables.size() == 2U &&
                selected_action_tables.back().first ==
                    FieldModeActionRecordTable::dynamic_menu_mode &&
                selected_action_tables.back().second == 2 &&
                glyph_triples == 47)) {
        return 11;
    }

    FieldModeSelectionPresentationState high_zero{};
    high_zero.mode_result = 0;
    high_zero.menu_mode = 5;
    high_zero.primary_cursor = static_cast<std::int8_t>(0x80U);
    render_field_mode_selection_presentation(high_zero, callbacks);
    if (!expect(selected_action_tables.size() == 3U &&
                selected_action_tables.back().first ==
                    FieldModeActionRecordTable::fixed_low_menu &&
                glyph_triples == 55)) {
        return 34;
    }

    return 0;
}
