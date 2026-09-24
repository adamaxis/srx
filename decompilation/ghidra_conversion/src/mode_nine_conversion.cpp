#include "shadowrun_ghidra/mode_nine_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void initialize_mode_nine(
    ModeNineInitializationState& state,
    const ModeNineInitializationCallbacks& callbacks) {
    if (callbacks.prepare_mode_entry) {
        callbacks.prepare_mode_entry();
    }
    if (callbacks.select_mode_resource) {
        callbacks.select_mode_resource();
    }

    state.mode = 9U;
    state.pending_resource = true;
    state.field_marker = 0xFFFFU;
    state.hud_mode = 3U;
    state.input_mode = 1U;
    state.scene_state_pending = true;
}

} // namespace shadowrun::ghidra_conversion
