#include "shadowrun_ghidra/field_mode_selection_presentation_conversion.hpp"

#include <array>
#include <cstdint>
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
    const FieldModeSelectionPresentationCallbacks callbacks{
        [&] { ++grids; },
        [&] { ++glyph_triples; },
        [&] { ++text_passes; },
        [&](const MenuResourceRecord& resource) {
            selected_resources.push_back(resource);
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

    FieldModeSelectionPresentationState negative_glyphs{};
    negative_glyphs.mode_result = -1;
    negative_glyphs.primary_cursor = 4;
    render_field_mode_selection_presentation(negative_glyphs, callbacks);
    if (!expect(glyph_triples == 8 && text_passes == 1)) {
        return 4;
    }

    FieldModeSelectionPresentationState result_one{};
    result_one.mode_result = 1;
    result_one.primary_cursor = 7;
    render_field_mode_selection_presentation(result_one, callbacks);
    if (!expect(grids == 16)) {
        return 5;
    }

    FieldModeSelectionPresentationState result_two{};
    result_two.mode_result = 2;
    result_two.menu_mode = 6;
    result_two.primary_cursor = 2;
    render_field_mode_selection_presentation(result_two, callbacks);
    if (!expect(grids == 24 && selected_resources.size() == 1U &&
                selected_resources.back().family ==
                    MenuResourceFamily::menu_mode_six_records &&
                selected_resources.back().record_offset == 0x20U)) {
        return 6;
    }

    result_two.primary_cursor = 5;
    render_field_mode_selection_presentation(result_two, callbacks);
    if (!expect(grids == 32)) {
        return 7;
    }
    result_two.primary_cursor = 4;
    render_field_mode_selection_presentation(result_two, callbacks);
    if (!expect(grids == 40)) {
        return 8;
    }

    FieldModeSelectionPresentationState normal{};
    normal.mode_result = 0;
    normal.primary_cursor = 0;
    render_field_mode_selection_presentation(normal, callbacks);
    if (!expect(glyph_triples == 16 && text_passes == 1)) {
        return 9;
    }
    normal.primary_cursor = 3;
    render_field_mode_selection_presentation(normal, callbacks);
    if (!expect(glyph_triples == 24 && text_passes == 1)) {
        return 10;
    }

    return 0;
}
