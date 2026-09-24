#include "shadowrun_ghidra/field_resource_sequence_conversion.hpp"

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
    const FieldViewportPreparationCallbacks& callbacks) {
    if (callbacks.prepare_scene_resources) {
        callbacks.prepare_scene_resources();
    }
    if (callbacks.refresh_mode_state) {
        callbacks.refresh_mode_state();
    }
    if (callbacks.special_effects_enabled &&
        callbacks.special_effects_enabled() &&
        callbacks.service_special_effects) {
        callbacks.service_special_effects();
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
