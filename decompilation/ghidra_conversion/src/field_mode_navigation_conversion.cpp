#include "shadowrun_ghidra/field_mode_navigation_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

template <typename Callback>
void call_if_present(const Callback& callback) {
    if (callback) {
        callback();
    }
}

std::int8_t clamp_cursor(const std::int8_t value, const std::int8_t maximum) {
    if (value < 0) {
        return 0;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

} // namespace

void refresh_field_mode_panel_grid(
    const FieldModeNavigationState& state,
    const FieldModeNavigationCallbacks& callbacks) {
    call_if_present(callbacks.prepare_selection_data);
    if (state.mode_result != 1) {
        call_if_present(callbacks.write_selection_grid);
    }
}

void render_field_mode_selection(
    const FieldModeNavigationState& state,
    const FieldModeNavigationCallbacks& callbacks) {
    // The source skips both writes only for result 1; all other source
    // result values take this normal redraw path.
    if (state.mode_result != 1) {
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
    // records, then call the remaining sprite-selection service.
    state.sprite_records_dirty = true;
    state.sprite_list_dirty = true;
    state.display_dirty_flags = static_cast<std::uint8_t>(
        state.display_dirty_flags | 0x02U);
    call_if_present(callbacks.build_selection_sprites);
}

void advance_field_mode_primary_cursor(
    FieldModeNavigationState& state,
    const std::int8_t signed_delta,
    const FieldModeNavigationCallbacks& callbacks) {
    const auto maximum = state.panel_cursor == 0
                             ? (state.mode_result == 0 ? std::int8_t{3}
                                                       : std::int8_t{5})
                             : std::int8_t{7};
    state.primary_cursor = clamp_cursor(
        static_cast<std::int8_t>(state.primary_cursor + signed_delta), maximum);
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
    state.panel_cursor = clamp_cursor(
        static_cast<std::int8_t>(state.panel_cursor + signed_delta), 1);
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
