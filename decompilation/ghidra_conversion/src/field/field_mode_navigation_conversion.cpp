#include "shadowrun_ghidra/field/field_mode_navigation_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

template <typename Callback>
void call_if_present(const Callback& callback) {
    if (callback) {
        callback();
    }
}

std::int8_t add_source_byte(const std::int8_t value,
                            const std::int8_t signed_delta) {
    // 0x58C54/0x58CDE use ADD.B.  Convert the wrapped low byte back to a
    // signed value before applying the source BPL/BMI tests.
    return static_cast<std::int8_t>(
        static_cast<std::uint8_t>(value) +
        static_cast<std::uint8_t>(signed_delta));
}

} // namespace

FieldModePanelGridPlan plan_field_mode_panel_grid(
    const std::int8_t mode_result) {
    FieldModePanelGridPlan plan{};
    if (mode_result < 0) {
        plan.variant = FieldModePanelGridVariant::negative_result;
        plan.destination_word = static_cast<std::int16_t>(-0x17F0);
        plan.grid_write_count = 0x12U;
    } else if (mode_result == 0) {
        plan.variant = FieldModePanelGridVariant::zero_result;
        plan.destination_word = static_cast<std::int16_t>(-0x17EA);
        plan.grid_write_count = 0x0CU;
    } else if (mode_result == 1) {
        plan.variant = FieldModePanelGridVariant::result_one_suppressed;
        plan.writes_grid = false;
    } else {
        plan.variant = FieldModePanelGridVariant::positive_result;
        plan.destination_word = static_cast<std::int16_t>(-0x17F0);
        plan.grid_write_count = 0x12U;
    }
    return plan;
}

FieldModeSelectionRenderPlan plan_field_mode_selection_render(
    const std::int8_t mode_result,
    const std::int8_t primary_cursor) {
    FieldModeSelectionRenderPlan plan{};
    plan.primary_cursor = primary_cursor;
    const auto cursor = static_cast<std::uint8_t>(primary_cursor);
    plan.grid_record_offset = static_cast<std::uint16_t>(cursor * 0x08U);
    plan.text_record_offset = static_cast<std::uint16_t>(cursor * 0x2AU);
    plan.screen_column_offset = static_cast<std::uint16_t>(cursor * 0x06U);
    if (mode_result < 0) {
        plan.family = FieldModeSelectionGridFamily::negative_result;
    } else if (mode_result == 0) {
        plan.family = FieldModeSelectionGridFamily::zero_result;
    } else if (mode_result == 1) {
        plan.family = FieldModeSelectionGridFamily::result_one_suppressed;
        plan.writes_grid_and_text = false;
    } else {
        plan.family = FieldModeSelectionGridFamily::positive_result;
    }
    return plan;
}

FieldModeSelectionSpritePlan plan_field_mode_selection_sprites(
    const std::int8_t primary_cursor) {
    FieldModeSelectionSpritePlan plan{};
    plan.primary_cursor = primary_cursor;
    plan.source_record_offset = static_cast<std::uint16_t>(
        static_cast<std::uint8_t>(primary_cursor) * 0x20U);
    return plan;
}

void refresh_field_mode_panel_grid(
    const FieldModeNavigationState& state,
    const FieldModeNavigationCallbacks& callbacks) {
    call_if_present(callbacks.prepare_selection_data);
    if (plan_field_mode_panel_grid(state.mode_result).writes_grid) {
        call_if_present(callbacks.write_selection_grid);
    }
}

void render_field_mode_selection(
    const FieldModeNavigationState& state,
    const FieldModeNavigationCallbacks& callbacks) {
    // The source skips both writes only for result 1; all other source
    // result values take this normal redraw path.
    if (plan_field_mode_selection_render(state.mode_result,
                                         state.primary_cursor)
            .writes_grid_and_text) {
        call_if_present(callbacks.write_selection_grid);
        call_if_present(callbacks.render_selection_text);
    }
}

namespace {

void refresh_normal_selection(
    const FieldModeNavigationState& state,
    const FieldModeNavigationCallbacks& callbacks) {
    refresh_field_mode_panel_grid(state, callbacks);
    render_field_mode_selection(state, callbacks);
}

} // namespace

void refresh_field_mode_selection_sprites(
    FieldModeNavigationState& state,
    const FieldModeNavigationCallbacks& callbacks) {
    // 0x58D32: set the sprite-list and display dirty markers, build the
    // records, then call 0x57C7A to redraw the action selection.
    const auto plan = plan_field_mode_selection_sprites(state.primary_cursor);
    state.sprite_records_dirty = true;
    state.sprite_list_dirty = true;
    state.display_dirty_flags = static_cast<std::uint8_t>(
        state.display_dirty_flags | 0x02U);
    if (callbacks.build_selection_sprites_detailed) {
        callbacks.build_selection_sprites_detailed(plan);
    } else {
        call_if_present(callbacks.build_selection_sprites);
    }
    call_if_present(callbacks.render_action_selection);
}

void advance_field_mode_primary_cursor(
    FieldModeNavigationState& state,
    const std::int8_t signed_delta,
    const FieldModeNavigationCallbacks& callbacks) {
    const auto maximum = state.panel_cursor == 0
                             ? (state.mode_result == 0 ? std::int8_t{3}
                                                       : std::int8_t{5})
                             : std::int8_t{7};
    const auto candidate = add_source_byte(state.primary_cursor, signed_delta);
    if (candidate < 0) {
        state.primary_cursor = 0;
        return;
    }
    if (candidate >= maximum + 1) {
        state.primary_cursor = maximum;
        return;
    }
    state.primary_cursor = candidate;
    if (state.panel_cursor != 0) {
        refresh_field_mode_panel_grid(state, callbacks);
        refresh_field_mode_selection_sprites(state, callbacks);
        return;
    }
    refresh_normal_selection(state, callbacks);
}

void advance_field_mode_panel_cursor(
    FieldModeNavigationState& state,
    const std::int8_t signed_delta,
    const FieldModeNavigationCallbacks& callbacks) {
    const auto candidate = add_source_byte(state.panel_cursor, signed_delta);
    if (candidate < 0) {
        state.panel_cursor = 0;
        return;
    }
    if (candidate >= 2) {
        state.panel_cursor = 1;
        return;
    }
    state.panel_cursor = candidate;
    state.primary_cursor = 0;
    if (state.panel_cursor != 0) {
        refresh_field_mode_panel_grid(state, callbacks);
        refresh_field_mode_selection_sprites(state, callbacks);
        return;
    }
    call_if_present(callbacks.reset_upload_state);
    refresh_normal_selection(state, callbacks);
}

} // namespace shadowrun::ghidra_conversion
