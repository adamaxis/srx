#pragma once

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

struct FieldViewportPreparationCallbacks {
    std::function<void()> prepare_scene_resources;
    std::function<void()> refresh_mode_state;
    std::function<bool()> special_effects_enabled;
    std::function<void()> service_special_effects;
    std::function<void()> render_normal_viewport;
    std::function<void()> render_alternate_viewport;
};

// ROM 0x2C6C: prepare the scene, service the optional mode branch, and submit
// both viewport variants. Rendering remains an explicit host boundary.
void prepare_field_viewports(
    const FieldViewportPreparationCallbacks& callbacks);

// ROM 0x33AA: one field-grid draw boundary.
void draw_field_asset_grid(const std::function<void()>& draw_grid);

} // namespace shadowrun::ghidra_conversion
