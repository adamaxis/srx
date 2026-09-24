#include "shadowrun_ghidra/mode_services_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void initialize_scene_mode_state(
    SceneModeInitializationState& state,
    const SceneModeInitializationCallbacks& callbacks) {
    state.current_mode = callbacks.prepare_mode_entry
        ? callbacks.prepare_mode_entry()
        : 0U;
    state.scene_transition_value = 0U;
    state.scene_mode_marker = 0xFFU;
    state.camera_state = 0xFFFFU;
    state.special_mode_flag = 0U;
    state.special_mode_indicator =
        static_cast<std::uint8_t>(state.current_mode == 6U);
}

void finalize_mode_exit(ModeExitState& state,
                        const ModeExitCallbacks& callbacks) {
    state.exit_service_marker = 0U;
    if (callbacks.service_mode_exit) {
        callbacks.service_mode_exit();
    }
}

void service_mode_world_state(
    ModeWorldRefreshState& state,
    const ModeWorldRefreshCallbacks& callbacks) {
    state.random_samples = 0U;
    if (callbacks.sample_random) {
        for (std::uint8_t sample = 0; sample < 4U; ++sample) {
            callbacks.sample_random();
            ++state.random_samples;
        }
    }
    state.grid_write_requested = true;
    if (callbacks.write_world_grid) {
        callbacks.write_world_grid();
    }
}

} // namespace shadowrun::ghidra_conversion
