#include "shadowrun_ghidra/title_resource_sequence_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void load_title_resources(const TitleResourceLoadCallbacks& callbacks) {
    if (callbacks.upload_resource_stream) {
        callbacks.upload_resource_stream();
    }
    if (callbacks.reset_resource_stream) {
        callbacks.reset_resource_stream();
    }
    if (callbacks.upload_resource_stream) {
        callbacks.upload_resource_stream();
        callbacks.upload_resource_stream();
    }
    if (callbacks.prepare_sprite_list) {
        callbacks.prepare_sprite_list();
    }
}

} // namespace shadowrun::ghidra_conversion
