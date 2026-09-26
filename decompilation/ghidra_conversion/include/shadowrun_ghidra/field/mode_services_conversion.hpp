#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct SceneModeInitializationState {
    std::uint8_t current_mode = 0;
    std::uint16_t scene_transition_value = 0;
    std::uint8_t scene_mode_marker = 0;
    std::uint16_t camera_state = 0;
    std::uint8_t special_mode_flag = 0;
    std::uint8_t special_mode_indicator = 0;
};

struct SceneModeInitializationCallbacks {
    std::function<std::uint8_t()> prepare_mode_entry;
};

// ROM 0x136DE: install the mode-entry result and clear the scene-mode
// transient markers. Mode six receives the source's special initialization.
void initialize_scene_mode_state(
    SceneModeInitializationState& state,
    const SceneModeInitializationCallbacks& callbacks);

struct ModeExitState {
    std::uint8_t exit_service_marker = 0;
};

struct ModeExitCallbacks {
    std::function<void()> service_mode_exit;
};

// ROM 0x20714: clear the exit-service marker and enter the existing exit
// service boundary at 0x1F246.
void finalize_mode_exit(ModeExitState& state,
                        const ModeExitCallbacks& callbacks);

struct ModeWorldRefreshState {
    std::uint8_t random_samples = 0;
    bool grid_write_requested = false;
};

struct ModeWorldRefreshCallbacks {
    std::function<void()> sample_random;
    std::function<void()> write_world_grid;
};

// ROM 0x0E6F4: consume the four source random samples and request the world
// grid write. The random generator and grid writer remain host boundaries.
void service_mode_world_state(
    ModeWorldRefreshState& state,
    const ModeWorldRefreshCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
