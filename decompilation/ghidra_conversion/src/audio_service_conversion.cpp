#include "shadowrun_ghidra/audio_service_conversion.hpp"

namespace shadowrun::ghidra_conversion {

bool advance_audio_service_tick(
    AudioServiceTickState& state,
    const bool audio_enabled,
    const AudioServiceCallbacks& callbacks) {
    state.phase = static_cast<std::uint8_t>(state.phase + 1U);
    return service_audio_if_enabled(audio_enabled, callbacks);
}

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

void activate_entity_audio_state(
    EntityAudioState& state,
    const std::function<void()>& service_audio) {
    state.flags = static_cast<std::uint8_t>(state.flags | 0x80U);
    state.behavior_timer =
        state.behavior_variant == 0x0EU ? 0x0010U : 0x000CU;
    state.update_marker = 0U;
    state.destination_x = state.current_x;
    state.destination_y = state.current_y;
    state.effect_word = 0U;
    if (service_audio) {
        service_audio();
    }
}

} // namespace shadowrun::ghidra_conversion
