#include "shadowrun_ghidra/field/field_mode_selection_presentation_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

template <typename Callback, typename... Args>
void call_if_present(const Callback& callback, Args&&... args) {
    if (callback) {
        callback(static_cast<Args&&>(args)...);
    }
}

void repeat(const std::size_t count,
            const std::function<void()>& callback) {
    for (std::size_t index = 0; index < count; ++index) {
        call_if_present(callback);
    }
}

} // namespace

MenuResourceRecord select_menu_resource(
    const std::uint8_t menu_mode,
    const std::int8_t saved_selection) {
    return {
        menu_mode == 6U
            ? MenuResourceFamily::menu_mode_six_records
            : MenuResourceFamily::general_menu_records,
        static_cast<std::uint16_t>(
            static_cast<std::uint8_t>(saved_selection) * 0x10U),
    };
}

MenuResourceEntry select_menu_resource_entry(
    const std::uint8_t menu_mode,
    const std::int8_t saved_selection,
    const std::int8_t primary_cursor) {
    return {select_menu_resource(menu_mode, saved_selection),
            static_cast<std::uint16_t>(
                static_cast<std::uint8_t>(primary_cursor) * 2U)};
}

void render_alternating_field_mode_records(
    const std::array<FieldModeSelectionGlyphRecord, 8>& records,
    const FieldModeSelectionPresentationCallbacks& callbacks) {
    for (const auto& record : records) {
        if (record.marker < 0 || record.marker != 'E') {
            call_if_present(callbacks.transfer_menu_glyph_triple);
        } else {
            call_if_present(callbacks.write_grid);
        }
    }
}

void render_field_mode_glyph_row(
    const std::int8_t saved_selection,
    const FieldModeSelectionPresentationCallbacks& callbacks) {
    repeat(8U, callbacks.transfer_menu_glyph_triple);
    if (saved_selection == 4) {
        call_if_present(callbacks.render_encoded_text);
    }
}

void render_field_mode_grid_pair_batches(
    const FieldModeSelectionPresentationCallbacks& callbacks) {
    repeat(6U, callbacks.write_grid);
    repeat(2U, callbacks.write_grid);
}

FieldModeNegativeGlyphTable negative_glyph_table_for_cursor(
    const std::int8_t primary_cursor) {
    switch (primary_cursor) {
    case 0:
        return FieldModeNegativeGlyphTable::cursor_zero;
    case 1:
        return FieldModeNegativeGlyphTable::cursor_one;
    default:
        return FieldModeNegativeGlyphTable::cursor_two;
    }
}

std::size_t negative_glyph_table_index(
    const FieldModeNegativeGlyphTable table) {
    return static_cast<std::size_t>(table);
}

void render_field_mode_selection_presentation(
    FieldModeSelectionPresentationState& state,
    const FieldModeSelectionPresentationCallbacks& callbacks) {
    state.saved_selection = state.primary_cursor;

    if (state.mode_result < 0) {
        if (state.primary_cursor < 3) {
            const auto table = negative_glyph_table_for_cursor(
                state.primary_cursor);
            if (callbacks.select_negative_glyph_table) {
                callbacks.select_negative_glyph_table(table, state.menu_mode);
            }
            render_alternating_field_mode_records(
                state.negative_glyph_records[negative_glyph_table_index(table)],
                callbacks);
            return;
        }
        if (state.primary_cursor == 3) {
            render_field_mode_grid_pair_batches(callbacks);
        } else {
            render_field_mode_glyph_row(state.saved_selection, callbacks);
        }
        return;
    }

    if (state.mode_result != 0) {
        if (state.mode_result == 1) {
            repeat(8U, callbacks.write_grid);
            return;
        }
        if (state.primary_cursor >= 3) {
            if (state.primary_cursor == 3) {
                repeat(8U, callbacks.write_grid);
            } else if (state.primary_cursor == 5) {
                repeat(5U, callbacks.write_grid);
                repeat(3U, callbacks.write_grid);
            } else {
                repeat(4U, callbacks.write_grid);
                repeat(4U, callbacks.write_grid);
            }
            return;
        }
        call_if_present(callbacks.select_resource,
                        select_menu_resource(state.menu_mode,
                                             state.saved_selection));
        repeat(8U, callbacks.write_grid);
        return;
    }

    if (state.primary_cursor == 0) {
        render_field_mode_glyph_row(state.saved_selection, callbacks);
        return;
    }
    if (state.primary_cursor == 3) {
        render_field_mode_glyph_row(state.saved_selection, callbacks);
        return;
    }
    if (callbacks.select_action_record_table) {
        if (state.saved_selection == 1) {
            callbacks.select_action_record_table(
                FieldModeActionRecordTable::fixed_selection_one, 0);
        } else if (state.saved_selection < 0) {
            callbacks.select_action_record_table(
                FieldModeActionRecordTable::dynamic_menu_mode,
                static_cast<std::int16_t>(state.menu_mode) - 3);
        } else {
            callbacks.select_action_record_table(
                FieldModeActionRecordTable::fixed_low_menu, 0);
        }
    }
    render_alternating_field_mode_records(state.glyph_records, callbacks);
}

} // namespace shadowrun::ghidra_conversion
