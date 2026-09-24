#include "shadowrun_ghidra/audio_service_conversion.hpp"

namespace shadowrun::ghidra_conversion {

bool service_audio_command_queue(const AudioServiceCallbacks& callbacks) {
    if (!callbacks.begin_bus_access || !callbacks.flush_audio_command ||
        !callbacks.append_audio_command || !callbacks.end_bus_access) {
        return false;
    }

    callbacks.begin_bus_access();
    callbacks.flush_audio_command();
    callbacks.append_audio_command();
    callbacks.end_bus_access();
    return true;
}

bool service_audio_if_enabled(
    const bool audio_enabled,
    const AudioServiceCallbacks& callbacks) {
    return audio_enabled && service_audio_command_queue(callbacks);
}

} // namespace shadowrun::ghidra_conversion
