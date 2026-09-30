#include "shadowrun_ghidra/field/mode_nine_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    ModeNineInitializationState state{
        false, 2U, 0x1234U, 7, 0, false, false};
    std::vector<int> order;
    initialize_mode_nine(
        state,
        {
            [&order] { order.push_back(1); },
            [&order] { order.push_back(2); },
        });

    if (!state.mode_nine_resource_selected || state.world_scene_mode != 9U ||
        state.field_mode_service_state != 0U ||
        state.camera_transition_marker != -1 ||
        state.field_mode_transition_index != 3 ||
        !state.alternate_scene_profile_selected ||
        !state.scene_profile_override_lock ||
        order != std::vector<int>{1, 2}) {
        return 1;
    }

    initialize_mode_nine(state, {});
    if (state.world_scene_mode != 9U || state.camera_transition_marker != -1 ||
        state.field_mode_transition_index != 3) {
        return 2;
    }
    return 0;
}
