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

enum class FieldModePanelGridVariant {
    negative_result,
    zero_result,
    result_one_suppressed,
    positive_result,
};

struct FieldModePanelGridPlan {
    FieldModePanelGridVariant variant =
        FieldModePanelGridVariant::zero_result;
    bool writes_grid = true;
    std::int16_t destination_word = 0;
    std::uint16_t grid_write_count = 0;
};

enum class FieldModeSelectionGridFamily {
    negative_result,
    zero_result,
    result_one_suppressed,
    positive_result,
};

struct FieldModeSelectionRenderPlan {
    FieldModeSelectionGridFamily family =
        FieldModeSelectionGridFamily::zero_result;
    bool writes_grid_and_text = true;
    std::int8_t primary_cursor = 0;
    std::uint16_t grid_record_offset = 0;
    std::uint16_t text_record_offset = 0;
    std::uint16_t screen_column_offset = 0;
};

struct FieldModeSelectionSpritePlan {
    std::int8_t primary_cursor = 0;
    std::uint16_t source_record_offset = 0;
    std::uint16_t sprite_build_counter = 2;
    std::uint16_t sprite_palette_attribute = 0x4000U;
    bool clears_trailing_marker = true;
    bool renders_action_selection_after_build = true;
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
    std::function<void(const FieldModeSelectionSpritePlan&)>
        build_selection_sprites_detailed;
    // 0x58D32 calls 0x57C7A after the sprite records and dirty markers are
    // published. This is separate from the later 0x587DA presentation pass.
    std::function<void()> render_action_selection;
};

// ROM 0x58B5E: select the result-specific panel grid asset, destination, and
// DBF-expanded write count after 0x57C46 prepares the selection data.
FieldModePanelGridPlan plan_field_mode_panel_grid(std::int8_t mode_result);

// ROM 0x58BB6: select the result-specific grid/text record families and apply
// the primary cursor's source strides (8 bytes, 0x2A bytes, and six screen
// words). Result one suppresses both writes.
FieldModeSelectionRenderPlan plan_field_mode_selection_render(
    std::int8_t mode_result,
    std::int8_t primary_cursor);

// ROM 0x58D32: derive the sprite-record source row and post-build actions.
FieldModeSelectionSpritePlan plan_field_mode_selection_sprites(
    std::int8_t primary_cursor);

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
