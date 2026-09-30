#include "shadowrun_ghidra/field/field_resource_sequence_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void prepare_field_resources(
    const FieldResourcePreparationCallbacks& callbacks) {
    if (callbacks.prepare_asset_setup) {
        callbacks.prepare_asset_setup();
    }
    if (callbacks.upload_asset_sequence) {
        callbacks.upload_asset_sequence();
    }
    if (callbacks.refresh_animation_assets) {
        callbacks.refresh_animation_assets();
    }
    if (callbacks.load_font) {
        callbacks.load_font();
    }
}

void prepare_field_viewports(
    const FieldViewportPreparationState& state,
    const FieldViewportPreparationCallbacks& callbacks) {
    if (callbacks.prepare_title_resources) {
        callbacks.prepare_title_resources();
    }
    if (callbacks.copy_mode_selection_state_table) {
        callbacks.copy_mode_selection_state_table();
    }
    if (state.field_mode == 2U && callbacks.upload_mode_word_block) {
        callbacks.upload_mode_word_block();
    }
    if (callbacks.render_normal_viewport) {
        callbacks.render_normal_viewport();
    }
    if (callbacks.render_alternate_viewport) {
        callbacks.render_alternate_viewport();
    }
}

void draw_field_asset_grid(const std::function<void()>& draw_grid) {
    if (draw_grid) {
        draw_grid();
    }
}

} // namespace shadowrun::ghidra_conversion
