#pragma once

#include <array>
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

struct AudioCommandQueueState {
    std::array<std::uint8_t, 64> bytes{};
    std::uint8_t write_index = 0;
};

void flush_audio_command(AudioCommandQueueState& state,
                         std::uint8_t command);
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
    std::function<void()> service_tick;
    std::function<void()> wait_for_vblank;
    std::function<void()> flush_queue;
    std::function<void()> finalize;
};

// ROM 0x53126: run six service/wait passes, then flush and finalize mode
// entry. The host owns the actual audio backend and timing source.
void prepare_audio_mode_entry(const AudioModeEntryCallbacks& callbacks);

bool service_audio_tick_wrapper(const AudioCommandSequenceCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
