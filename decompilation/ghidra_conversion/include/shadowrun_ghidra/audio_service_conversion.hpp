#pragma once

#include <functional>

namespace shadowrun::ghidra_conversion {

struct AudioServiceCallbacks {
    std::function<void()> begin_bus_access;
    std::function<void()> flush_audio_command;
    std::function<void()> append_audio_command;
    std::function<void()> end_bus_access;
};

// ROM 0x52F3C: service the audio command queue in its source order. The
// queue contents and Z80 bus are host-owned boundaries rather than emulated
// memory or processor state.
bool service_audio_command_queue(const AudioServiceCallbacks& callbacks);

// ROM 0x530DE: perform the queue service only when the source audio-enable
// flag is set.
bool service_audio_if_enabled(bool audio_enabled,
                              const AudioServiceCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
