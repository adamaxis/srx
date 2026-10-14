#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct FieldActionSelectionNavigationState {
    // FFF0B7: negative = action detail/availability presentation, zero =
    // ordinary field-action records, one = equipment upgrade, positive =
    // Matrix/field resource purchase presentation.
    std::int8_t menu_result = 0;
    std::int8_t action_cursor = 0;  // FFF0D0: selected field-action row
    std::int8_t action_panel = 0;   // FFF0D1: normal/alternate action panel
    std::int8_t selected_action = 0; // FFF0B8: action handed to 0x56DC4
    bool sprite_records_dirty = false; // FFDD02
    bool sprite_list_dirty = false;    // FFD980
    std::uint8_t display_dirty_flags = 0; // FFEF14
};

enum class FieldActionSelectionPanelGridVariant {
    detail_result,
    ordinary_action_result,
    equipment_upgrade_result,
    resource_purchase_result,
};

struct FieldActionSelectionPanelGridPlan {
    FieldActionSelectionPanelGridVariant variant =
        FieldActionSelectionPanelGridVariant::ordinary_action_result;
    bool writes_grid = true;
    std::int16_t destination_word = 0;
    std::uint16_t grid_write_count = 0;
};

enum class FieldActionSelectionGridFamily {
    detail_result,
    ordinary_action_result,
    equipment_upgrade_result,
    resource_purchase_result,
};

struct FieldActionSelectionRenderPlan {
    FieldActionSelectionGridFamily family =
        FieldActionSelectionGridFamily::ordinary_action_result;
    bool writes_grid_and_text = true;
    std::int8_t action_cursor = 0;
    std::uint16_t grid_record_offset = 0;
    std::uint16_t text_record_offset = 0;
    std::uint16_t screen_column_offset = 0;
};

struct FieldActionSelectionSpritePlan {
    std::int8_t action_cursor = 0;
    std::uint16_t source_record_offset = 0;
    std::uint16_t sprite_build_counter = 2;
    std::uint16_t sprite_palette_attribute = 0x4000U;
    bool clears_trailing_marker = true;
    bool renders_action_selection_after_build = true;
};

struct FieldActionSelectionNavigationCallbacks {
    // 0x57C46 prepares the current selection's source data. Its actual table
    // is still a separate extraction boundary.
    std::function<void()> prepare_field_action_selection_data;
    std::function<void()> write_field_action_selection_grid;
    std::function<void()> render_field_action_selection_text;

    // 0x58D32 builds the moving selection sprite records and advances the
    // source sprite-selection service.
    std::function<void()> build_field_action_cursor_sprites;
    std::function<void()> reset_upload_state;
    std::function<void(const FieldActionSelectionSpritePlan&)>
        build_field_action_cursor_sprites_detailed;
    // 0x58D32 calls 0x57C7A after the sprite records and dirty markers are
    // published. This is separate from the later 0x587DA presentation pass.
    std::function<void()> render_field_action_selection;
};

// ROM 0x58B5E: select the field-action panel grid for the current result
// family (detail, ordinary action, equipment upgrade, or resource purchase),
// destination, and DBF-expanded write count after 0x57C46 prepares data.
FieldActionSelectionPanelGridPlan plan_field_action_selection_panel_grid(
    std::int8_t menu_result);

// ROM 0x58BB6: select the field-action grid/text record families and apply
// the action cursor's source strides (8 bytes, 0x2A bytes, and six screen
// words). The equipment-upgrade result suppresses both normal-panel writes.
FieldActionSelectionRenderPlan plan_field_action_selection_render(
    std::int8_t menu_result,
    std::int8_t action_cursor);

// ROM 0x58D32: derive the sprite-record source row and post-build actions.
FieldActionSelectionSpritePlan plan_field_action_selection_sprites(
    std::int8_t action_cursor);

// 0x58B5E: refresh only the detail/resource grid portion used by the
// alternate field-action panel.
void refresh_field_action_selection_panel_grid(
    const FieldActionSelectionNavigationState& state,
    const FieldActionSelectionNavigationCallbacks& callbacks);

// 0x58BB6: write the normal field-action grid and its encoded text. The
// source caller invokes 0x58B5E immediately before this leaf when both
// passes are required.
void render_field_action_selection_panel(
    const FieldActionSelectionNavigationState& state,
    const FieldActionSelectionNavigationCallbacks& callbacks);

// 0x58C54: apply the byte-wrapped source cursor delta. In panel 0 the bound
// is 3 when menu_result is zero and 5 otherwise; the alternate panel has a
// bound of 7. Negative and upper-bound clamps return without redrawing;
// only an in-range cursor write enters the corresponding refresh path.
void advance_field_action_selection_cursor(
    FieldActionSelectionNavigationState& state,
    std::int8_t signed_delta,
    const FieldActionSelectionNavigationCallbacks& callbacks);

// 0x58CDE: apply the byte-wrapped two-position field-action panel selector.
// An in-range change resets the action-row cursor and refreshes the
// corresponding normal or alternate presentation; a clamp returns without
// that refresh.
void advance_field_action_selection_panel(
    FieldActionSelectionNavigationState& state,
    std::int8_t signed_delta,
    const FieldActionSelectionNavigationCallbacks& callbacks);

// 0x58D32: publish the source sprite/HUD dirty markers before handing off to
// the unresolved sprite-selection service.
void refresh_field_action_selection_sprites(
    FieldActionSelectionNavigationState& state,
    const FieldActionSelectionNavigationCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
