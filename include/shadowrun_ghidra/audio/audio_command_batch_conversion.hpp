#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct AudioBusBoundaryCallbacks {
    std::function<void()> begin;
    std::function<void()> end;
};

void begin_controller_bus_access(const AudioBusBoundaryCallbacks& callbacks);
void end_controller_bus_access(const AudioBusBoundaryCallbacks& callbacks);
void begin_audio_bus_access(const AudioBusBoundaryCallbacks& callbacks);
void end_audio_bus_access(const AudioBusBoundaryCallbacks& callbacks);

struct AudioRamState {
    std::vector<std::uint8_t> ram;
    bool reset_asserted = false;
};

// ROM 0x52DE0: load the source audio image into a fixed host-owned 0x2000
// byte workspace and clear the remainder.
void initialize_audio_ram(AudioRamState& state,
                          const std::vector<std::uint8_t>& source,
                          const AudioBusBoundaryCallbacks& controller_bus);

struct AudioResetCallbacks {
    std::function<void()> assert_reset;
    std::function<void()> wait_short_delay;
    std::function<void()> release_bus;
    std::function<void()> release_reset;
};

// ROM 0x52E26: perform the source reset pulse without exposing Z80 registers.
void reset_audio_processor(AudioRamState& state,
                           const AudioResetCallbacks& callbacks);

enum class AudioQueueEntryKind {
    empty,
    command,
    flush_marker,
};

struct AudioQueueEntry {
    AudioQueueEntryKind kind = AudioQueueEntryKind::empty;
    std::uint8_t command = 0;
};

struct AudioCommandQueueState {
    static constexpr std::size_t capacity = 64U;
    std::array<AudioQueueEntry, capacity> entries{};
    std::size_t write_index = 0;
};

// ROM 0x52E96: write a typed flush marker, append the command through the
// shared 0x52EA2 body, and wrap the six-bit queue index. The source's 0xFF
// marker is represented by AudioQueueEntryKind::flush_marker, not exposed as
// a magic byte in the host model.
void flush_audio_command(AudioCommandQueueState& state,
                         std::uint8_t command);
// ROM 0x52EA2: append one typed command and wrap the six-bit queue index.
// This is also the inner entry reached by the 0x52E96 body.
void append_audio_command(AudioCommandQueueState& state,
                          std::uint8_t command);

struct AudioCommandSequenceCallbacks {
    std::function<void()> begin;
    std::function<void()> end;
    std::function<void()> flush;
    std::function<void()> append;
};

bool append_one_with_audio_bus(const AudioCommandSequenceCallbacks& callbacks);
bool append_three_with_audio_bus(const AudioCommandSequenceCallbacks& callbacks);
bool service_audio_command_queue_batch(
    const AudioCommandSequenceCallbacks& callbacks);
bool flush_with_audio_bus(const AudioCommandSequenceCallbacks& callbacks);
// ROM 0x52FD2: append a marker-prefixed command and two payload bytes. The
// source routine is entered after its caller has opened the audio bus; it does
// not open the bus itself.
bool flush_append_pair(const AudioCommandSequenceCallbacks& callbacks);
bool begin_bus_then_flush_append_pair(
    const AudioCommandSequenceCallbacks& callbacks);
bool controller_access_pair(const AudioBusBoundaryCallbacks& callbacks);

bool service_audio_if_enabled_variant(
    bool audio_enabled,
    const AudioCommandSequenceCallbacks& callbacks);

struct AudioFrameCallbacks {
    std::function<void()> prepare_command;
    AudioCommandSequenceCallbacks queue;
};

// ROM 0x530EA: prepare and service one enabled audio command frame.
bool service_audio_command_frame_variant(bool audio_enabled,
                                         const AudioFrameCallbacks& callbacks);

bool service_audio_command_wrapper(
    const AudioCommandSequenceCallbacks& callbacks);

struct AudioModeEntryCallbacks {
    // ROM 0x5312C: D0 is initialized to the fixed mode-entry command 0x14
    // before each 0x5315E transaction wrapper.
    std::function<void(std::uint32_t command)> run_fixed_audio_transaction;
    // ROM 0x53138: D5 is set to 2, so 0x0C24 performs three vblank waits.
    std::function<void(std::uint16_t source_counter)> wait_for_vblank_frames;
    // ROM 0x53146/0x5314C: flush the audio bus, then run the zero-command
    // service wrapper.
    std::function<void()> flush_audio_bus;
    std::function<void()> service_audio_tick;
};

// ROM 0x53126: run six 0x14 audio transactions, each followed by a three-
// vblank wait, then flush the bus and service the zero-command audio tick.
// The host owns the actual audio backend and timing source.
void prepare_audio_mode_entry(const AudioModeEntryCallbacks& callbacks);

bool service_audio_tick_wrapper(const AudioCommandSequenceCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
