#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct AudioServiceCallbacks {
    std::function<void()> begin_bus_access;
    std::function<void()> flush_audio_command;
    std::function<void()> append_audio_command;
    std::function<void()> end_bus_access;
};

struct AudioServiceTickState {
    std::uint8_t phase = 0;
};

// ROM 0x17D3E: increment the byte-sized audio phase before servicing audio.
bool advance_audio_service_tick(
    AudioServiceTickState& state,
    bool audio_enabled,
    const AudioServiceCallbacks& callbacks);

// ROM 0x52F3C: service the audio command queue in its source order. The
// queue contents and Z80 bus are host-owned boundaries rather than emulated
// memory or processor state.
bool service_audio_command_queue(const AudioServiceCallbacks& callbacks);

// ROM 0x530DE: perform the queue service only when the source audio-enable
// flag is set.
bool service_audio_if_enabled(bool audio_enabled,
                              const AudioServiceCallbacks& callbacks);

struct EntityAudioState {
    std::uint8_t flags = 0;
    std::uint8_t behavior_variant = 0;
    std::uint8_t update_marker = 0;
    std::uint16_t behavior_timer = 0;
    std::uint16_t current_x = 0;
    std::uint16_t current_y = 0;
    std::uint16_t destination_x = 0;
    std::uint16_t destination_y = 0;
    std::uint16_t effect_word = 0;
};

// ROM 0x15354: arm an entity's audio/action record and invoke the audio
// service boundary after copying its current position.
void activate_entity_audio_state(
    EntityAudioState& state,
    const std::function<void()>& service_audio);

} // namespace shadowrun::ghidra_conversion
