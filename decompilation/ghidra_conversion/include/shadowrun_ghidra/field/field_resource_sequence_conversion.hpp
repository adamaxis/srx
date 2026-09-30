#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct FieldResourcePreparationCallbacks {
    std::function<void()> prepare_asset_setup;
    std::function<void()> upload_asset_sequence;
    std::function<void()> refresh_animation_assets;
    std::function<void()> load_font;
};

// ROM 0x28AA: prepare the field's extracted resources in source order.
void prepare_field_resources(
    const FieldResourcePreparationCallbacks& callbacks);

struct FieldViewportPreparationState {
    // ROM FFDF40: only field mode 2 submits the extra mode word block.
    std::uint16_t field_mode = 0;
};

struct FieldViewportPreparationCallbacks {
    // ROM 0x1406: shared title/field resource preparation.
    std::function<void()> prepare_title_resources;
    // ROM 0x566C2: copy the 24-word mode-selection state table.
    std::function<void()> copy_mode_selection_state_table;
    // ROM 0x16AFA: upload the mode-specific word block when FFDF40 == 2.
    std::function<void()> upload_mode_word_block;
    std::function<void()> render_normal_viewport;
    std::function<void()> render_alternate_viewport;
};

// ROM 0x2C6C: prepare title resources, copy the mode-selection table, upload
// the mode word block only for field mode 2, then submit normal and alternate
// viewport variants. Rendering remains an explicit host boundary.
void prepare_field_viewports(
    const FieldViewportPreparationState& state,
    const FieldViewportPreparationCallbacks& callbacks);

// ROM 0x33AA: one field-grid draw boundary.
void draw_field_asset_grid(const std::function<void()>& draw_grid);

} // namespace shadowrun::ghidra_conversion
