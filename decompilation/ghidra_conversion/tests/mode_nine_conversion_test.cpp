#include "shadowrun_ghidra/mode_nine_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    ModeNineInitializationState state{
        2U, false, 0x1234U, 7U, 0U, false};
    std::vector<int> order;
    initialize_mode_nine(
        state,
        {
            [&order] { order.push_back(1); },
            [&order] { order.push_back(2); },
        });

    if (state.mode != 9U || !state.pending_resource ||
        state.field_marker != 0xFFFFU || state.hud_mode != 3U ||
        state.input_mode != 1U || !state.scene_state_pending ||
        order != std::vector<int>{1, 2}) {
        return 1;
    }

    initialize_mode_nine(state, {});
    if (state.mode != 9U || state.field_marker != 0xFFFFU) {
        return 2;
    }
    return 0;
}
