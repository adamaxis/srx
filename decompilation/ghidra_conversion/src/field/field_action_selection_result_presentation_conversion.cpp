#include "shadowrun_ghidra/field/field_action_selection_result_presentation_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

bool source_cmp_byte_is_negative(const std::int8_t value,
                                 const std::uint8_t immediate) {
    // 0x587DA uses CMP.B followed by BMI at its cursor/table boundaries.
    // Preserve the wrapped N bit instead of applying a host signed ordering.
    return (static_cast<std::uint8_t>(value) - immediate) & 0x80U;
}

FieldActionSelectionGlyphPalettePlane glyph_palette_for_marker(
    const std::uint8_t marker) {
    if ((marker & 0x80U) != 0U) {
        // 0x58A2E takes the negative-marker path to the blank glyph and the
        // transfer call without running the #0x0F comparison.
        return FieldActionSelectionGlyphPalettePlane::high_0x6000;
    }
    return source_cmp_byte_is_negative(static_cast<std::int8_t>(marker),
                                       0x0FU)
               ? FieldActionSelectionGlyphPalettePlane::low_0x2000
               : FieldActionSelectionGlyphPalettePlane::high_0x6000;
}

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

FieldActionSelectionRecordPresentationPlan plan_field_action_selection_record(
    const std::uint8_t marker) {
    FieldActionSelectionRecordPresentationPlan plan{};
    plan.marker = marker;
    plan.writes_grid = marker == static_cast<std::uint8_t>('E');
    plan.glyph_palette = glyph_palette_for_marker(marker);
    return plan;
}

FieldActionSelectionInventoryGlyphPlan
plan_field_action_selection_inventory_glyph(
    const std::uint8_t item_code) {
    FieldActionSelectionInventoryGlyphPlan plan{};
    plan.item_code = item_code;
    plan.uses_blank_asset = item_code == 0U;
    if (!plan.uses_blank_asset) {
        plan.item_definition_index = static_cast<std::uint8_t>(item_code - 1U);
    }
    return plan;
}

FieldActionSelectionPositiveGridPlan plan_field_action_selection_positive_grid(
    const std::uint8_t menu_mode,
    const std::int8_t selected_action,
    const std::int8_t action_cursor,
    const std::int8_t menu_result) {
    FieldActionSelectionPositiveGridPlan plan{};
    plan.menu_mode = menu_mode;
    plan.action_cursor = action_cursor;
    if (menu_result == 1) {
        plan.kind =
            FieldActionSelectionPositiveGridPlanKind::equipment_upgrade_result;
        plan.first_grid_count = 8U;
        return plan;
    }
    if (source_cmp_byte_is_negative(action_cursor, 3U)) {
        plan.kind = FieldActionSelectionPositiveGridPlanKind::
            matrix_program_resource_description;
        plan.first_grid_count = 8U;
        plan.selects_menu_resource = true;
        plan.selected_resource = select_menu_resource(menu_mode,
                                                      selected_action);
        return plan;
    }
    if (action_cursor == 3) {
        plan.kind = FieldActionSelectionPositiveGridPlanKind::
            field_resource_record_purchase_rows;
        plan.resource_table_family =
            menu_mode == 6U
                ? FieldActionSelectionPositiveResourceTableFamily::
                      mode_six_resource_records
                : FieldActionSelectionPositiveResourceTableFamily::
                      other_mode_resource_records;
        plan.resource_table_offset = menu_mode == 6U ? 0U : 0x70U;
        plan.first_grid_count = 8U;
        return plan;
    }
    if (action_cursor == 5) {
        plan.kind = FieldActionSelectionPositiveGridPlanKind::
            field_quantity_purchase_split;
        plan.first_grid_count = 5U;
        plan.second_grid_count = 3U;
        return plan;
    }
    plan.kind = FieldActionSelectionPositiveGridPlanKind::field_resource_purchase_split;
    plan.first_grid_count = 4U;
    plan.second_grid_count = 4U;
    return plan;
}

FieldActionSelectionResourceGridRowPlan
plan_field_action_selection_resource_grid_row(
    const std::uint8_t marker) {
    FieldActionSelectionResourceGridRowPlan plan{};
    plan.marker = marker;
    plan.uses_blank_grid = (marker & 0x80U) != 0U;
    if (!plan.uses_blank_grid) {
        plan.description_record_offset =
            static_cast<std::uint16_t>(marker * 0x0CU);
    }
    return plan;
}

FieldActionSelectionPositiveResourceRowPlan
plan_field_action_selection_positive_resource_row(
    const std::uint8_t marker) {
    return {marker, (marker & 0x80U) != 0U};
}

MenuResourceRecord select_menu_resource(
    const std::uint8_t menu_mode,
    const std::int8_t selected_action) {
    return {
        menu_mode == 6U
            ? MenuResourceFamily::menu_mode_six_records
            : MenuResourceFamily::general_menu_records,
        static_cast<std::uint16_t>(
            static_cast<std::uint8_t>(selected_action) * 0x10U),
    };
}

MenuResourceEntry select_menu_resource_entry(
    const std::uint8_t menu_mode,
    const std::int8_t selected_action,
    const std::int8_t action_cursor) {
    return {select_menu_resource(menu_mode, selected_action),
            static_cast<std::uint16_t>(
                static_cast<std::uint8_t>(action_cursor) * 2U)};
}

void render_alternating_field_action_selection_records(
    const std::array<FieldActionSelectionGlyphRecord, 8>& records,
    const FieldActionSelectionPresentationCallbacks& callbacks) {
    for (std::size_t index = 0; index < records.size(); ++index) {
        const auto& record = records[index];
        const auto plan = plan_field_action_selection_record(
            static_cast<std::uint8_t>(record.marker));
        if (!plan.writes_grid) {
            if (callbacks.transfer_record_glyph) {
                callbacks.transfer_record_glyph(index, plan);
            } else {
                call_if_present(callbacks.transfer_menu_glyph_triple);
            }
        } else {
            if (callbacks.write_record_grid) {
                callbacks.write_record_grid(index, plan);
            } else {
                call_if_present(callbacks.write_grid);
            }
        }
    }
}

void render_field_action_selection_inventory_glyph_row(
    const std::int8_t selected_action,
    const FieldActionSelectionPresentationCallbacks& callbacks) {
    render_field_action_selection_inventory_glyph_row(
        std::array<std::uint8_t, 8>{}, selected_action, callbacks);
}

void render_field_action_selection_inventory_glyph_row(
    const std::array<std::uint8_t, 8>& inventory_item_codes,
    const std::int8_t selected_action,
    const FieldActionSelectionPresentationCallbacks& callbacks) {
    for (std::size_t index = 0; index < inventory_item_codes.size(); ++index) {
        const auto plan = plan_field_action_selection_inventory_glyph(
            inventory_item_codes[index]);
        if (callbacks.transfer_inventory_glyph) {
            callbacks.transfer_inventory_glyph(index, plan);
        } else {
            call_if_present(callbacks.transfer_menu_glyph_triple);
        }
    }
    if (selected_action == 4) {
        call_if_present(callbacks.render_encoded_text);
    }
}

void render_field_action_selection_grid_pair_batches(
    const FieldActionSelectionPresentationCallbacks& callbacks) {
    repeat(6U, callbacks.write_grid);
    repeat(2U, callbacks.write_grid);
}

FieldActionSelectionDetailGlyphTable action_detail_glyph_table_for_cursor(
    const std::int8_t action_cursor) {
    switch (action_cursor) {
    case 0:
        return FieldActionSelectionDetailGlyphTable::action_detail_cursor_zero;
    case 1:
        return FieldActionSelectionDetailGlyphTable::action_detail_cursor_one;
    default:
        return FieldActionSelectionDetailGlyphTable::action_detail_cursor_two;
    }
}

std::size_t action_detail_glyph_table_index(
    const FieldActionSelectionDetailGlyphTable table) {
    return static_cast<std::size_t>(table);
}

void render_field_action_selection_presentation(
    FieldActionSelectionPresentationState& state,
    const FieldActionSelectionPresentationCallbacks& callbacks) {
    state.selected_action = state.action_cursor;

    if (state.menu_result < 0) {
        if (state.action_cursor == 0 ||
            source_cmp_byte_is_negative(state.action_cursor, 2U) ||
            state.action_cursor == 2) {
            const auto table = action_detail_glyph_table_for_cursor(
                state.action_cursor);
            if (callbacks.select_action_detail_glyph_table) {
                callbacks.select_action_detail_glyph_table(table,
                                                           state.menu_mode);
            }
            render_alternating_field_action_selection_records(
                state.action_detail_glyph_records[action_detail_glyph_table_index(table)],
                callbacks);
            return;
        }
        if (state.action_cursor == 3) {
            render_field_action_selection_grid_pair_batches(callbacks);
        } else {
            render_field_action_selection_inventory_glyph_row(
                state.inventory_item_codes, state.selected_action, callbacks);
        }
        return;
    }

    if (state.menu_result != 0) {
        const auto plan = plan_field_action_selection_positive_grid(
            state.menu_mode, state.selected_action, state.action_cursor,
            state.menu_result);
        call_if_present(callbacks.select_positive_grid_plan, plan);
        if (plan.selects_menu_resource) {
            call_if_present(callbacks.select_resource, plan.selected_resource);
        }
        if (plan.kind == FieldActionSelectionPositiveGridPlanKind::
                           field_resource_record_purchase_rows) {
            for (std::size_t index = 0; index < 8U; ++index) {
                const auto row =
                    plan_field_action_selection_positive_resource_row(
                        state.positive_resource_markers[index]);
                if (callbacks.write_positive_resource_row) {
                    callbacks.write_positive_resource_row(index, row);
                } else {
                    call_if_present(callbacks.write_grid);
                }
            }
        } else if (plan.kind == FieldActionSelectionPositiveGridPlanKind::
                                      matrix_program_resource_description) {
            for (std::size_t index = 0; index < 8U; ++index) {
                const auto row = plan_field_action_selection_resource_grid_row(
                    state.resource_grid_markers[index]);
                if (callbacks.write_resource_grid_row) {
                    callbacks.write_resource_grid_row(index, row);
                } else {
                    call_if_present(callbacks.write_grid);
                }
            }
        } else {
            repeat(plan.first_grid_count, callbacks.write_grid);
            repeat(plan.second_grid_count, callbacks.write_grid);
        }
        return;
    }

    if (state.action_cursor == 3) {
        render_field_action_selection_inventory_glyph_row(
            state.inventory_item_codes, state.selected_action, callbacks);
        return;
    }
    if (callbacks.select_action_record_table) {
        if (state.selected_action == 1) {
            callbacks.select_action_record_table(
                FieldActionSelectionActionRecordTable::fixed_selection_one, 0);
        } else if (source_cmp_byte_is_negative(state.selected_action, 1U)) {
            callbacks.select_action_record_table(
                FieldActionSelectionActionRecordTable::dynamic_menu_mode,
                static_cast<std::int16_t>(state.menu_mode) - 3);
        } else {
            callbacks.select_action_record_table(
                FieldActionSelectionActionRecordTable::fixed_low_menu, 0);
        }
    }
    render_alternating_field_action_selection_records(state.glyph_records,
                                                      callbacks);
}

} // namespace shadowrun::ghidra_conversion
