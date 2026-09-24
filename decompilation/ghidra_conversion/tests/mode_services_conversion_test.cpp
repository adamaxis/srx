#include "shadowrun_ghidra/mode_services_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    SceneModeInitializationState scene{};
    int prepared = 0;
    SceneModeInitializationCallbacks scene_callbacks{};
    scene_callbacks.prepare_mode_entry = [&] {
        ++prepared;
        return static_cast<std::uint8_t>(6U);
    };
    initialize_scene_mode_state(scene, scene_callbacks);
    if (prepared != 1 || scene.current_mode != 6U ||
        scene.scene_transition_value != 0U || scene.scene_mode_marker != 0xFFU ||
        scene.camera_state != 0xFFFFU || scene.special_mode_flag != 0U ||
        scene.special_mode_indicator != 1U) {
        return 1;
    }

    ModeExitState exit_state{7U};
    int exit_calls = 0;
    ModeExitCallbacks exit_callbacks{};
    exit_callbacks.service_mode_exit = [&] { ++exit_calls; };
    finalize_mode_exit(exit_state, exit_callbacks);
    if (exit_state.exit_service_marker != 0U || exit_calls != 1) {
        return 2;
    }

    ModeWorldRefreshState world{};
    int random_calls = 0;
    int grid_calls = 0;
    ModeWorldRefreshCallbacks world_callbacks{};
    world_callbacks.sample_random = [&] { ++random_calls; };
    world_callbacks.write_world_grid = [&] { ++grid_calls; };
    service_mode_world_state(world, world_callbacks);
    if (world.random_samples != 4U || random_calls != 4 ||
        !world.grid_write_requested || grid_calls != 1) {
        return 3;
    }
    return 0;
}
