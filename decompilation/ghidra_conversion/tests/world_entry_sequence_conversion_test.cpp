#include "shadowrun_ghidra/world_entry_sequence_conversion.hpp"

#include <vector>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    WorldEntrySequenceState state{};
    state.world_origin_x = 100;
    state.world_origin_y = 200;
    std::vector<int> order;
    std::size_t selected_streams = 0U;
    const WorldEntrySequenceCallbacks callbacks{
        [&](const bool compact_layout) {
            ++selected_streams;
            if (compact_layout) {
                return WorldEntryCommandStream{1U, {0x10U, 0x02U}, {}};
            }
            return WorldEntryCommandStream{
                1U, {0xF1U, 0x10U, 0x00U, 0x11U, 0x01U}, {1U}};
        },
        [&] { order.push_back(1); },
        [&] { order.push_back(2); },
        [&] { order.push_back(3); },
        [&] { order.push_back(4); },
        [&] { order.push_back(5); },
        [&] { order.push_back(6); },
        [&] { order.push_back(7); },
        [&] { order.push_back(8); }};

    if (expect(run_world_entry_sequence(state, false, callbacks) ==
                   WorldEntrySequenceAction::completed &&
               selected_streams == 1U && state.animation_marker == -1 &&
               state.world_origin_x == 93 && state.world_origin_y == 199 &&
               state.accumulated_x == 2 && state.accumulated_y == 1 &&
               state.display_transfer_pending && state.sprite_records_pending &&
               order.size() == 48U && order[0] == 1 && order[1] == 2 &&
               order[2] == 3 && order[3] == 4 && order[11] == 4 &&
               order[12] == 1 && order[23] == 4 && order[32] == 1 &&
               order[43] == 4 && order[44] == 5 && order[45] == 6 &&
               order[46] == 7 && order[47] == 8) != 0) {
        return 1;
    }

    state = {};
    if (expect(run_world_entry_sequence(state, true, callbacks) ==
                   WorldEntrySequenceAction::completed &&
               state.accumulated_x == 1 && state.accumulated_y == 2) != 0) {
        return 2;
    }

    const WorldEntrySequenceCallbacks invalid_callbacks{
        [](bool) { return WorldEntryCommandStream{1U, {}, {}}; }};
    if (expect(run_world_entry_sequence(state, false, invalid_callbacks) ==
                   WorldEntrySequenceAction::invalid_stream) != 0) {
        return 3;
    }
    return 0;
}
