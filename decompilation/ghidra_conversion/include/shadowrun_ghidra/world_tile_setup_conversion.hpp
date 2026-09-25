#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct WorldTileResourceRecord {
    std::uint16_t origin_x = 0;
    std::uint16_t origin_y = 0;
};

enum class WorldTileDispatchStage {
    primary_buffer,
    secondary_buffer,
};

struct WorldTileSetupState {
    WorldTileResourceRecord initial_origin{}; // FFE192/FFE194
    WorldTileResourceRecord current_origin{}; // FFE19A/FFE19C
    std::uint16_t viewport_width = 0; // FFE19E
    std::uint16_t viewport_height = 0; // FFE1A0
    std::uint16_t accumulated_x = 0; // FFE1A2
    std::uint16_t accumulated_y = 0; // FFE1A4
    std::uint16_t tile_step_x = 1; // FFE1A6
    std::uint16_t tile_step_y = 1; // FFE1A8
    std::int16_t pending_buffer_records = 0; // FFB740 first word
    std::uint16_t world_origin_x = 0; // FFDD28
    std::uint16_t world_origin_y = 0; // FFDD2A
};

struct WorldTileSetupCallbacks {
    std::function<WorldTileResourceRecord()> select_resource_record;
    std::function<void()> copy_tile_blocks;
    std::function<void(WorldTileDispatchStage)> dispatch_tile_buffer;
};

// ROM 0x266E4: initialize the typed world tile-buffer state and run the
// resource/copy/primary-buffer/secondary-buffer service sequence.
void initialize_world_tile_buffers(
    WorldTileSetupState& state,
    bool compact_layout,
    const WorldTileSetupCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
