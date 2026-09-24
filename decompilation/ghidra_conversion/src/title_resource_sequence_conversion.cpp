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

void prepare_title_sprite_list(const std::function<void()>& prepare_sprites) {
    if (prepare_sprites) {
        prepare_sprites();
    }
}

} // namespace shadowrun::ghidra_conversion
