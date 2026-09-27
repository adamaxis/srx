#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct FieldModeNavigationState {
    std::int8_t mode_result = 0;      // FFF0B7
    std::int8_t primary_cursor = 0;   // FFF0D0
    std::int8_t panel_cursor = 0;     // FFF0D1
    std::int8_t saved_selection = 0;  // FFF0B8
    bool sprite_records_dirty = false; // FFDD02
    bool sprite_list_dirty = false;    // FFD980
    std::uint8_t display_dirty_flags = 0; // FFEF14
};

struct FieldModeNavigationCallbacks {
    // 0x57C46 prepares the current selection's source data. Its actual table
    // is still a separate extraction boundary.
    std::function<void()> prepare_selection_data;
    std::function<void()> write_selection_grid;
    std::function<void()> render_selection_text;

    // 0x58D32 builds the moving selection sprite records and advances the
    // source sprite-selection service.
    std::function<void()> build_selection_sprites;
    std::function<void()> reset_upload_state;
};

// 0x58B5E: refresh only the grid portion used by the alternate panel.
void refresh_field_mode_panel_grid(
    const FieldModeNavigationState& state,
    const FieldModeNavigationCallbacks& callbacks);

// 0x58BB6: write the normal selection grid and its encoded text. The source
// caller invokes 0x58B5E immediately before this leaf when both passes are
// required.
void render_field_mode_selection(
    const FieldModeNavigationState& state,
    const FieldModeNavigationCallbacks& callbacks);

// 0x58C54: apply the byte-wrapped source cursor delta. In panel 0 the bound
// is 3 when mode_result is zero and 5 otherwise; the alternate panel has a
// bound of 7. Negative and upper-bound clamps return without redrawing;
// only an in-range cursor write enters the corresponding refresh path.
void advance_field_mode_primary_cursor(
    FieldModeNavigationState& state,
    std::int8_t signed_delta,
    const FieldModeNavigationCallbacks& callbacks);

// 0x58CDE: apply the byte-wrapped two-position panel cursor. An in-range
// change resets the primary cursor and refreshes the corresponding normal or
// alternate presentation; a clamp returns without that refresh.
void advance_field_mode_panel_cursor(
    FieldModeNavigationState& state,
    std::int8_t signed_delta,
    const FieldModeNavigationCallbacks& callbacks);

// 0x58D32: publish the source sprite/HUD dirty markers before handing off to
// the unresolved sprite-selection service.
void refresh_field_mode_selection_sprites(
    FieldModeNavigationState& state,
    const FieldModeNavigationCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
