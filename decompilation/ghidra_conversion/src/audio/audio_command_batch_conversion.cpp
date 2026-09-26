#include "shadowrun_ghidra/audio/audio_command_batch_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

bool valid_sequence(const AudioCommandSequenceCallbacks& callbacks) {
    return static_cast<bool>(callbacks.begin) && static_cast<bool>(callbacks.end) &&
           static_cast<bool>(callbacks.flush) &&
           static_cast<bool>(callbacks.append);
}

} // namespace

void begin_controller_bus_access(
    const AudioBusBoundaryCallbacks& callbacks) {
    call_once(callbacks.begin);
}

void end_controller_bus_access(const AudioBusBoundaryCallbacks& callbacks) {
    call_once(callbacks.end);
}

void begin_audio_bus_access(const AudioBusBoundaryCallbacks& callbacks) {
    call_once(callbacks.begin);
}

void end_audio_bus_access(const AudioBusBoundaryCallbacks& callbacks) {
    call_once(callbacks.end);
}

void initialize_audio_ram(AudioRamState& state,
                          const std::vector<std::uint8_t>& source,
                          const AudioBusBoundaryCallbacks& controller_bus) {
    state.reset_asserted = true;
    begin_controller_bus_access(controller_bus);
    state.ram.assign(0x2000U, 0);
    const auto copy_count = std::min<std::size_t>(0x1886U, source.size());
    std::copy_n(source.begin(), copy_count, state.ram.begin());
}

void reset_audio_processor(AudioRamState& state,
                           const AudioResetCallbacks& callbacks) {
    state.reset_asserted = true;
    call_once(callbacks.assert_reset);
    for (int delay = 0; delay < 16; ++delay) {
        call_once(callbacks.wait_short_delay);
    }
    call_once(callbacks.release_bus);
    state.reset_asserted = false;
    call_once(callbacks.release_reset);
}

void flush_audio_command(AudioCommandQueueState& state,
                         const std::uint8_t command) {
    const auto marker_index = state.write_index & 0x3FU;
    state.bytes[marker_index] = 0xFFU;
    state.bytes[(marker_index + 1U) & 0x3FU] = command;
    state.write_index = static_cast<std::uint8_t>((marker_index + 2U) & 0x3FU);
}

void append_audio_command(AudioCommandQueueState& state,
                          const std::uint8_t command) {
    state.bytes[state.write_index & 0x3FU] = command;
    state.write_index = static_cast<std::uint8_t>((state.write_index + 1U) &
                                                   0x3FU);
}

bool append_one_with_audio_bus(
    const AudioCommandSequenceCallbacks& callbacks) {
    if (!valid_sequence(callbacks)) {
        return false;
    }
    callbacks.begin();
    callbacks.append();
    callbacks.end();
    return true;
}

bool append_three_with_audio_bus(
    const AudioCommandSequenceCallbacks& callbacks) {
    if (!valid_sequence(callbacks)) {
        return false;
    }
    callbacks.begin();
    callbacks.append();
    callbacks.append();
    callbacks.append();
    callbacks.end();
    return true;
}

bool service_audio_command_queue_batch(
    const AudioCommandSequenceCallbacks& callbacks) {
    if (!valid_sequence(callbacks)) {
        return false;
    }
    callbacks.begin();
    callbacks.flush();
    callbacks.append();
    callbacks.end();
    return true;
}

bool flush_with_audio_bus(const AudioCommandSequenceCallbacks& callbacks) {
    if (!valid_sequence(callbacks)) {
        return false;
    }
    callbacks.begin();
    callbacks.flush();
    callbacks.end();
    return true;
}

bool flush_append_pair(const AudioCommandSequenceCallbacks& callbacks) {
    if (!valid_sequence(callbacks)) {
        return false;
    }
    callbacks.flush();
    callbacks.append();
    callbacks.append();
    callbacks.end();
    return true;
}

bool begin_bus_then_flush_append_pair(
    const AudioCommandSequenceCallbacks& callbacks) {
    if (!valid_sequence(callbacks)) {
        return false;
    }
    callbacks.begin();
    return flush_append_pair(callbacks);
}

bool controller_access_pair(const AudioBusBoundaryCallbacks& callbacks) {
    if (!callbacks.begin || !callbacks.end) {
        return false;
    }
    callbacks.begin();
    callbacks.end();
    return true;
}

bool service_audio_if_enabled_variant(
    const bool audio_enabled,
    const AudioCommandSequenceCallbacks& callbacks) {
    return audio_enabled && service_audio_command_queue_batch(callbacks);
}

bool service_audio_command_frame_variant(
    const bool audio_enabled,
    const AudioFrameCallbacks& callbacks) {
    if (!audio_enabled) {
        return false;
    }
    call_once(callbacks.prepare_command);
    return service_audio_command_queue_batch(callbacks.queue);
}

bool service_audio_command_wrapper(
    const AudioCommandSequenceCallbacks& callbacks) {
    return flush_with_audio_bus(callbacks);
}

void prepare_audio_mode_entry(const AudioModeEntryCallbacks& callbacks) {
    for (int pass = 0; pass < 6; ++pass) {
        call_once(callbacks.service_tick);
        call_once(callbacks.wait_for_vblank);
    }
    call_once(callbacks.flush_queue);
    call_once(callbacks.finalize);
}

bool service_audio_tick_wrapper(
    const AudioCommandSequenceCallbacks& callbacks) {
    return flush_with_audio_bus(callbacks);
}

} // namespace shadowrun::ghidra_conversion
