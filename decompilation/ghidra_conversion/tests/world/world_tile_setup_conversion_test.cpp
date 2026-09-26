#include "shadowrun_ghidra/world/world_tile_setup_conversion.hpp"

#include <vector>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    WorldTileSetupState state{};
    state.pending_buffer_records = 4;
    int copies = 0;
    std::vector<WorldTileDispatchStage> dispatches;
    const WorldTileSetupCallbacks callbacks{
        [] { return WorldTileResourceRecord{0x1234U, 0x5678U}; },
        [&] { ++copies; },
        [&](const WorldTileDispatchStage stage) { dispatches.push_back(stage); }};

    initialize_world_tile_buffers(state, false, callbacks);
    if (expect(state.initial_origin.origin_x == 0x1234U &&
               state.initial_origin.origin_y == 0x5678U &&
               state.current_origin.origin_x == 0x1234U &&
               state.viewport_width == 14U && state.viewport_height == 8U &&
               state.accumulated_x == 0U && state.accumulated_y == 0U &&
               state.tile_step_x == 1U && state.tile_step_y == 1U &&
               state.pending_buffer_records == 3 && copies == 1 &&
               state.world_origin_x == 0x11EU &&
               state.world_origin_y == 0xE4U && dispatches.size() == 2U &&
               dispatches[0] == WorldTileDispatchStage::primary_buffer &&
               dispatches[1] == WorldTileDispatchStage::secondary_buffer) != 0) {
        return 1;
    }

    initialize_world_tile_buffers(state, true, {});
    if (expect(state.viewport_width == 12U && state.viewport_height == 10U &&
               state.pending_buffer_records == 2) != 0) {
        return 2;
    }
    return 0;
}
