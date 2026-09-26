#include "shadowrun_ghidra/field/mode_exit_resources_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void prepare_mode_exit_resources(
    ModeExitResourceState& state,
    const ModeExitResourceCallbacks& callbacks) {
    if (state.optional_resource_pending && callbacks.prepare_optional_resource) {
        callbacks.prepare_optional_resource();
    }
    if (callbacks.refresh_mode_resource_metadata) {
        callbacks.refresh_mode_resource_metadata();
    }
    if (callbacks.decode_and_upload_source_stream) {
        callbacks.decode_and_upload_source_stream();
    }
    if (state.asset_mode < 0) {
        if (state.resource_cursor < 0) {
            state.resource_cursor = 0;
        }
        if (callbacks.prepare_mode_resource_variant_a) {
            callbacks.prepare_mode_resource_variant_a();
        }
        if (callbacks.prepare_mode_resource_variant_b) {
            callbacks.prepare_mode_resource_variant_b();
        }
        state.asset_mode = 0;
    } else {
        if (callbacks.render_hud_sprite_stream) {
            callbacks.render_hud_sprite_stream();
        }
        state.hud_rendered = true;
    }
    state.optional_resource_pending = false;
    state.optional_resource_cleared = true;
}

} // namespace shadowrun::ghidra_conversion
