#include "shadowrun_ghidra/world/world_tile_setup_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void initialize_world_tile_buffers(
    WorldTileSetupState& state,
    const bool compact_layout,
    const WorldTileSetupCallbacks& callbacks) {
    state.initial_origin = callbacks.select_resource_record
        ? callbacks.select_resource_record()
        : WorldTileResourceRecord{};
    state.current_origin = state.initial_origin;
    state.viewport_width = compact_layout ? 12U : 14U;
    state.viewport_height = compact_layout ? 10U : 8U;
    state.accumulated_x = 0U;
    state.accumulated_y = 0U;
    state.tile_step_x = 1U;
    state.tile_step_y = 1U;

    if (callbacks.copy_tile_blocks) {
        callbacks.copy_tile_blocks();
    }
    if (callbacks.dispatch_tile_buffer) {
        callbacks.dispatch_tile_buffer(WorldTileDispatchStage::primary_buffer);
        callbacks.dispatch_tile_buffer(WorldTileDispatchStage::secondary_buffer);
    }
    --state.pending_buffer_records;
    state.world_origin_x = 0x11EU;
    state.world_origin_y = 0xE4U;
}

} // namespace shadowrun::ghidra_conversion
