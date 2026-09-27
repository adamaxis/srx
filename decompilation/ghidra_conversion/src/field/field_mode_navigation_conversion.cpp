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

FieldActionSelectionPanelGridPlan plan_field_action_selection_panel_grid(
    const std::int8_t menu_result) {
    FieldActionSelectionPanelGridPlan plan{};
    if (menu_result < 0) {
        plan.variant = FieldActionSelectionPanelGridVariant::detail_result;
        plan.destination_word = static_cast<std::int16_t>(-0x17F0);
        plan.grid_write_count = 0x12U;
    } else if (menu_result == 0) {
        plan.variant =
            FieldActionSelectionPanelGridVariant::ordinary_action_result;
        plan.destination_word = static_cast<std::int16_t>(-0x17EA);
        plan.grid_write_count = 0x0CU;
    } else if (menu_result == 1) {
        plan.variant =
            FieldActionSelectionPanelGridVariant::equipment_upgrade_result;
        plan.writes_grid = false;
    } else {
        plan.variant =
            FieldActionSelectionPanelGridVariant::resource_purchase_result;
        plan.destination_word = static_cast<std::int16_t>(-0x17F0);
        plan.grid_write_count = 0x12U;
    }
    return plan;
}

FieldActionSelectionRenderPlan plan_field_action_selection_render(
    const std::int8_t menu_result,
    const std::int8_t action_cursor) {
    FieldActionSelectionRenderPlan plan{};
    plan.action_cursor = action_cursor;
    const auto cursor = static_cast<std::uint8_t>(action_cursor);
    plan.grid_record_offset = static_cast<std::uint16_t>(cursor * 0x08U);
    plan.text_record_offset = static_cast<std::uint16_t>(cursor * 0x2AU);
    plan.screen_column_offset = static_cast<std::uint16_t>(cursor * 0x06U);
    if (menu_result < 0) {
        plan.family = FieldActionSelectionGridFamily::detail_result;
    } else if (menu_result == 0) {
        plan.family = FieldActionSelectionGridFamily::ordinary_action_result;
    } else if (menu_result == 1) {
        plan.family = FieldActionSelectionGridFamily::equipment_upgrade_result;
        plan.writes_grid_and_text = false;
    } else {
        plan.family = FieldActionSelectionGridFamily::resource_purchase_result;
    }
    return plan;
}

FieldActionSelectionSpritePlan plan_field_action_selection_sprites(
    const std::int8_t action_cursor) {
    FieldActionSelectionSpritePlan plan{};
    plan.action_cursor = action_cursor;
    plan.source_record_offset = static_cast<std::uint16_t>(
        static_cast<std::uint8_t>(action_cursor) * 0x20U);
    return plan;
}

void refresh_field_action_selection_panel_grid(
    const FieldActionSelectionNavigationState& state,
    const FieldActionSelectionNavigationCallbacks& callbacks) {
    call_if_present(callbacks.prepare_field_action_selection_data);
    if (plan_field_action_selection_panel_grid(state.menu_result).writes_grid) {
        call_if_present(callbacks.write_field_action_selection_grid);
    }
}

void render_field_action_selection_panel(
    const FieldActionSelectionNavigationState& state,
    const FieldActionSelectionNavigationCallbacks& callbacks) {
    // The source skips both writes only for result 1; all other source
    // result values take this normal redraw path.
    if (plan_field_action_selection_render(state.menu_result,
                                         state.action_cursor)
            .writes_grid_and_text) {
        call_if_present(callbacks.write_field_action_selection_grid);
        call_if_present(callbacks.render_field_action_selection_text);
    }
}

namespace {

void refresh_ordinary_field_action_selection(
    const FieldActionSelectionNavigationState& state,
    const FieldActionSelectionNavigationCallbacks& callbacks) {
    refresh_field_action_selection_panel_grid(state, callbacks);
    render_field_action_selection_panel(state, callbacks);
}

} // namespace

void refresh_field_action_selection_sprites(
    FieldActionSelectionNavigationState& state,
    const FieldActionSelectionNavigationCallbacks& callbacks) {
    // 0x58D32: set the sprite-list and display dirty markers, build the
    // records, then call 0x57C7A to redraw the action selection.
    const auto plan = plan_field_action_selection_sprites(state.action_cursor);
    state.sprite_records_dirty = true;
    state.sprite_list_dirty = true;
    state.display_dirty_flags = static_cast<std::uint8_t>(
        state.display_dirty_flags | 0x02U);
    if (callbacks.build_field_action_cursor_sprites_detailed) {
        callbacks.build_field_action_cursor_sprites_detailed(plan);
    } else {
        call_if_present(callbacks.build_field_action_cursor_sprites);
    }
    call_if_present(callbacks.render_field_action_selection);
}

void advance_field_action_selection_cursor(
    FieldActionSelectionNavigationState& state,
    const std::int8_t signed_delta,
    const FieldActionSelectionNavigationCallbacks& callbacks) {
    const auto maximum = state.action_panel == 0
                             ? (state.menu_result == 0 ? std::int8_t{3}
                                                       : std::int8_t{5})
                             : std::int8_t{7};
    const auto candidate = add_source_byte(state.action_cursor, signed_delta);
    if (candidate < 0) {
        state.action_cursor = 0;
        return;
    }
    if (candidate >= maximum + 1) {
        state.action_cursor = maximum;
        return;
    }
    state.action_cursor = candidate;
    if (state.action_panel != 0) {
        refresh_field_action_selection_panel_grid(state, callbacks);
        refresh_field_action_selection_sprites(state, callbacks);
        return;
    }
    refresh_ordinary_field_action_selection(state, callbacks);
}

void advance_field_action_selection_panel(
    FieldActionSelectionNavigationState& state,
    const std::int8_t signed_delta,
    const FieldActionSelectionNavigationCallbacks& callbacks) {
    const auto candidate = add_source_byte(state.action_panel, signed_delta);
    if (candidate < 0) {
        state.action_panel = 0;
        return;
    }
    if (candidate >= 2) {
        state.action_panel = 1;
        return;
    }
    state.action_panel = candidate;
    state.action_cursor = 0;
    if (state.action_panel != 0) {
        refresh_field_action_selection_panel_grid(state, callbacks);
        refresh_field_action_selection_sprites(state, callbacks);
        return;
    }
    call_if_present(callbacks.reset_upload_state);
    refresh_ordinary_field_action_selection(state, callbacks);
}

} // namespace shadowrun::ghidra_conversion
