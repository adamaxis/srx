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

bool valid_append_sequence(const AudioCommandSequenceCallbacks& callbacks) {
    return static_cast<bool>(callbacks.end) &&
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
    const auto marker_index =
        state.write_index % AudioCommandQueueState::capacity;
    state.entries[marker_index] =
        {AudioQueueEntryKind::flush_marker, 0U};
    state.entries[(marker_index + 1U) %
                  AudioCommandQueueState::capacity] =
        {AudioQueueEntryKind::command, command};
    state.write_index = (marker_index + 2U) %
                        AudioCommandQueueState::capacity;
}

void append_audio_command(AudioCommandQueueState& state,
                          const std::uint8_t command) {
    const auto command_index =
        state.write_index % AudioCommandQueueState::capacity;
    state.entries[command_index] =
        {AudioQueueEntryKind::command, command};
    state.write_index = (command_index + 1U) %
                        AudioCommandQueueState::capacity;
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
    if (!valid_append_sequence(callbacks)) {
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
    constexpr std::uint32_t kModeEntryCommand = 0x14U;
    constexpr std::uint16_t kModeEntryVblankCounter = 2U;
    constexpr std::size_t kModeEntryPassCount = 6U;
    for (std::size_t pass = 0; pass < kModeEntryPassCount; ++pass) {
        if (callbacks.run_fixed_audio_transaction) {
            callbacks.run_fixed_audio_transaction(kModeEntryCommand);
        }
        if (callbacks.wait_for_vblank_frames) {
            callbacks.wait_for_vblank_frames(kModeEntryVblankCounter);
        }
    }
    call_once(callbacks.flush_audio_bus);
    call_once(callbacks.service_audio_tick);
}

bool service_audio_tick_wrapper(
    const AudioCommandSequenceCallbacks& callbacks) {
    return flush_with_audio_bus(callbacks);
}

} // namespace shadowrun::ghidra_conversion
