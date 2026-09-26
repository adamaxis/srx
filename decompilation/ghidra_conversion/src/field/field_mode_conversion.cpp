#include "shadowrun_ghidra/field/field_mode_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void initialize_field_mode(
    FieldModeInitializationState& state,
    const FieldModeProfile profile,
    const FieldModeInitializationCallbacks& callbacks) {
    state.profile = profile;
    if (callbacks.select_profile_data) {
        callbacks.select_profile_data(profile);
    }
    if (callbacks.prepare_mode_entry) {
        callbacks.prepare_mode_entry();
    }
    state.transient_fields.fill(0U);
    state.render_pending = false;
    if (callbacks.initialize_scene_mode_state) {
        callbacks.initialize_scene_mode_state();
    }
}

} // namespace shadowrun::ghidra_conversion
