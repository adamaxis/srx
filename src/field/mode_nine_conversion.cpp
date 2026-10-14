#include "shadowrun_ghidra/field/mode_nine_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void initialize_mode_nine(
    ModeNineInitializationState& state,
    const ModeNineInitializationCallbacks& callbacks) {
    if (callbacks.prepare_mode_entry) {
        callbacks.prepare_mode_entry();
    }
    if (callbacks.select_mode_nine_resource) {
        callbacks.select_mode_nine_resource();
    }

    state.mode_nine_resource_selected = true;
    state.world_scene_mode = 9U;
    state.field_mode_service_state = 0U;
    state.camera_transition_marker = -1;
    state.field_mode_transition_index = 3;
    state.alternate_scene_profile_selected = true;
    state.scene_profile_override_lock = true;
}

} // namespace shadowrun::ghidra_conversion
