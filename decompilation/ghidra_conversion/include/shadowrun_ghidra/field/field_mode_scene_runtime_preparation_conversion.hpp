#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

// ROM 0xC7F8 chooses whether field-scene preparation is a runner-state asset
// refresh or the full field-mode exit finalizer. The source tests FF014C; the
// host keeps that value as a typed field-action counter instead of a memory
// lookup.
enum class FieldSceneRuntimePreparationPath {
    refresh_runner_state_and_assets,
    finalize_field_mode_exit_transition,
};

struct FieldSceneRuntimePreparationCallbacks {
    std::function<void()> refresh_runner_state_and_assets;
    std::function<void()> finalize_field_mode_exit_transition;
};

struct FieldSceneRuntimePreparationDecision {
    FieldSceneRuntimePreparationPath path =
        FieldSceneRuntimePreparationPath::finalize_field_mode_exit_transition;
    bool callback_dispatched = false;
};

// ROM 0xC7F8: a nonzero FF014C dispatches to 0xC802; zero dispatches to
// 0xC64C. The selected callee remains an explicit typed callback because
// those two routines own separate field-state contracts.
FieldSceneRuntimePreparationDecision prepare_field_scene_runtime(
    std::uint8_t field_action_counter,
    const FieldSceneRuntimePreparationCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
