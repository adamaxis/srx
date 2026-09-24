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

} // namespace shadowrun::ghidra_conversion
