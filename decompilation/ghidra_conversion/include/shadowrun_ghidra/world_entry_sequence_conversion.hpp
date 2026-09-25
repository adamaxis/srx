#pragma once

#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct WorldEntryCommandStream {
    std::uint16_t frame_count = 0;
    std::vector<std::uint8_t> movement_commands;
    std::vector<std::uint8_t> delays;
};

struct WorldEntrySequenceState {
    std::int8_t animation_marker = -1; // FFF0E1
    std::int32_t world_origin_x = 0; // FFDD28
    std::int32_t world_origin_y = 0; // FFDD2A
    std::int32_t accumulated_x = 0; // FFE1A2
    std::int32_t accumulated_y = 0; // FFE1A4
    bool display_transfer_pending = false; // FFDD02/FFEF14
    bool sprite_records_pending = false; // FFD980
};

struct WorldEntrySequenceCallbacks {
    std::function<WorldEntryCommandStream(bool compact_layout)>
        select_command_stream;
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<void()> advance_hud_sprite_audio_pulse;
    std::function<void()> advance_menu_random_glitch;
    std::function<void()> dispatch_frame_buffer;
    std::function<void()> write_frame_transfer;
    std::function<void()> advance_title_sprite_layers;
    std::function<void()> append_world_sprite_records;
};

enum class WorldEntrySequenceAction {
    completed,
    invalid_stream,
};

// ROM 0x2679E: run the extracted world-entry command stream while preserving
// source timing and the dispatch/transfer/sprite publication order.
WorldEntrySequenceAction run_world_entry_sequence(
    WorldEntrySequenceState& state,
    bool compact_layout,
    const WorldEntrySequenceCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
